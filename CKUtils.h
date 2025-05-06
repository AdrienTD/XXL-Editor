#pragma once

#include "KObject.h"
#include "KEnvironment.h"
#include <string>
#include "File.h"
#include <stack>
#include <nlohmann/json_fwd.hpp>

struct MemberListener;
struct Vector3;
struct Matrix;
struct File;
struct EventNode;
struct MarkerIndex;
struct Encyclopedia;

struct KAnyPostponedRef {
	kobjref<CKObject> ref;
	uint32_t id = 0xFFFFFFFF;
	kuuid uuid;
	bool bound = true;

	void read(File* file) {
		bound = false;
		id = file->readUint32();
		if (id == 0xFFFFFFFD)
			file->read(uuid.data(), 16);
	}

	void write(KEnvironment* kenv, File* file) const {
		if (bound) {
			kenv->writeObjRef(file, ref);
		}
		else {
			file->writeUint32(id);
			if (id == 0xFFFFFFFD)
				file->write(uuid.data(), 16);
		}
	}

	void bind(KEnvironment* kenv, int sector) {
		if (id == 0xFFFFFFFD)
			ref = kenv->getObjRef<CKObject>(uuid);
		else
			ref = kenv->getObjRef<CKObject>(id, sector);
		bound = true;
	}
};

// Reference that stores the serialized form of the reference,
// and only deserializes after a call to bind().
// Useful in cases where you have to read a reference to an object from STR,
// but you know the exact sector number only later in the level loading process.
template <class T> struct KPostponedRef : KAnyPostponedRef {
	void bind(KEnvironment* kenv, int sector) {
		KAnyPostponedRef::bind(kenv, sector);
		if (ref) {
			assert(ref->isSubclassOf<T>());
		}
	}

	T *get() const { assert(bound); return (T*)ref.get(); }
	T *operator->() const { return get(); }

	void set(T* obj) { ref = obj; bound = true; }
	KPostponedRef& operator=(T* obj) { set(obj); return *this; }

	//operator kobjref<T>&() { assert(bound); return ref; }
};

template<class U, class V> bool operator==(kobjref<U> &ref, KPostponedRef<V> &post) { return ref.get() == post.get(); }
template<class U, class V> bool operator==(KPostponedRef<V> &post, kobjref<U> &ref) { return ref.get() == post.get(); }

// Reading/writing member listeners

struct StructMemberListener;

// Class whose methods are called for every member of a KClass that reflects
// its members through a "reflectMembers" method. Useful to iterate through
// every member of a class that supports reflection, to load/save instances
// as well as display every member in the editor.
struct MemberListener {
	virtual void reflect(uint8_t& ref, const char* name) = 0;
	virtual void reflect(uint16_t& ref, const char* name) = 0;
	virtual void reflect(uint32_t& ref, const char* name) = 0;
	virtual void reflect(int8_t& ref, const char* name) { reflect((uint8_t&)ref, name); }
	virtual void reflect(int16_t& ref, const char* name) { reflect((uint16_t&)ref, name); }
	virtual void reflect(int32_t& ref, const char* name) { reflect((uint32_t&)ref, name); }
	virtual void reflect(float& ref, const char* name) = 0;
	virtual void reflectAnyRef(kanyobjref& ref, int clfid, const char* name) = 0;
	virtual void reflect(Vector3& ref, const char* name) = 0;
	virtual void reflect(Matrix& ref, const char* name);
	virtual void reflect(EventNode& ref, const char* name, CKObject* user /*= nullptr*/) = 0;
	virtual void reflect(MarkerIndex& ref, const char* name) = 0;
	virtual void reflectPostRefTuple(uint32_t& tuple, const char* name) { reflect(tuple, name); }
	virtual void reflectAnyPostRef(KAnyPostponedRef& postref, int clfid, const char* name) {
		if (postref.bound)
			reflectAnyRef(postref.ref, clfid, name);
		else
			reflectPostRefTuple(postref.id, name);
	}
	virtual void reflect(std::string& ref, const char* name) = 0;

	virtual void enterArray(const char* name) {}
	virtual void setNextIndex(int index) {}
	virtual void incrementIndex() {}
	virtual void leaveArray() {}
	virtual void enterStruct(const char* name) {}
	virtual void leaveStruct() {}

	virtual void message(const char* msg) {}

	using FileAccessor = void(*)(File* file, void* ctx);
	virtual void onReadImpl(void* ctx, FileAccessor fa) {}
	virtual void onWriteImpl(void* ctx, FileAccessor fa) {}
	template <typename T> void onRead (T* ctx, void(*fa)(File* file, T* ctx)) { onReadImpl(ctx, (FileAccessor)fa); }
	template <typename T> void onWrite(T* ctx, void(*fa)(File* file, T* ctx)) { onWriteImpl(ctx, (FileAccessor)fa); }

	enum class MemberFlags : int {
		MF_NONE = 0,
		MF_EDITOR_HIDDEN = 1,
		MF_EDITOR_PROTECTED = 2,
		MF_DUPLICATOR_IGNORED = 4,
		MF_DUPLICATOR_NULLIFY = 8,
		MF_HOOK_INTERNAL = MF_EDITOR_HIDDEN | MF_EDITOR_PROTECTED | MF_DUPLICATOR_IGNORED,
	};
	virtual void setNextFlags(MemberFlags flags) {}
	template <typename T> void reflectEx(T& ref, const char* name, MemberFlags flags) { setNextFlags(flags); reflect(ref, name); setNextFlags(MemberFlags::MF_NONE); }

	struct MinusFID {
		static constexpr int FULL_ID = -1;
	};
	template <class T> void reflect(kobjref<T> &ref, const char *name) {
		static constexpr int fid = std::conditional<std::is_same<T, CKObject>::value, MinusFID, T>::type::FULL_ID;
		reflectAnyRef(ref, fid, name);
	};
	template <class T> void reflect(KPostponedRef<T> &ref, const char *name) {
		static constexpr int fid = std::conditional<std::is_same<T, CKObject>::value, MinusFID, T>::type::FULL_ID;
		reflectAnyPostRef(ref, fid, name);
	}

	template <class T> void reflectContainer(T &ref, const char *name) {
		int i = 0;
		enterArray(name);
		for (auto &elem : ref) {
			setNextIndex(i++);
			reflect(elem, name);
		}
		leaveArray();
	}
	template <class T, size_t N> void reflect(std::array<T, N> &ref, const char *name) { reflectContainer(ref, name); }
	template <class T> void reflect(std::vector<T> &ref, const char *name) { reflectContainer(ref, name); }

	template <class A, class B> void reflect(std::pair<A, B>& ref, const char* name) {
		enterStruct(name);
		reflect(ref.first, "first");
		reflect(ref.second, "second");
		leaveStruct();
	}
	template<typename Tup, size_t ... I> void reflectTuple(Tup& ref, const char* name, std::index_sequence<I...>) {
		enterArray(name);
		//(reflect(std::get<I>(ref), ('_' + std::to_string(I)).c_str()), ...);
		((setNextIndex(I), reflect(std::get<I>(ref), name)), ...);
		leaveArray();
	}
	template<class ... A> void reflect(std::tuple<A...>& ref, const char* name) {
		reflectTuple(ref, name, std::make_index_sequence<sizeof...(A)>());
	}

	template <class T> void reflect(T &ref, const char *name) {
		static_assert(std::is_class<T>::value, "cannot be reflected");
		enterStruct(name);
		ref.reflectMembers(*this);
		leaveStruct();
	}
	template <class SizeInt, class T> void reflectSize(T &container, const char *name) {
		SizeInt siz, newsiz;
		siz = newsiz = (SizeInt)container.size();
		reflect(newsiz, name);
		if (newsiz != siz)
			container.resize(newsiz);
	}

	template <typename A, typename T> void reflectAs(T& ref, const char* name) {
		A conv = (A)ref;
		reflect(conv, name);
		ref = (T)conv;
	}

	template <typename T, typename Func> void foreachElement(T& container, const char* name, Func func) {
		int i = 0;
		enterArray(name);
		for (auto& elem : container) {
			setNextIndex(i++);
			enterStruct(name);
			func(elem);
			leaveStruct();
		}
		leaveArray();
	}

	template <typename T> void reflectComposed(T& inst, const char* name, KEnvironment* kenv) {
		enterStruct(name);
		inst.reflectMembers2(*this, kenv);
		leaveStruct();
	}
};

struct [[deprecated]] StructMemberListener : MemberListener {
	MemberListener &ml; std::string structName;
	StructMemberListener(MemberListener &ml, const char *structName) : ml(ml), structName(structName) {}
	std::string getMemberName(const char *memberName) { return structName + "." + memberName; }
	void reflect(uint8_t &ref, const char *name) override { ml.reflect(ref, getMemberName(name).c_str()); }
	void reflect(uint16_t &ref, const char *name) override { ml.reflect(ref, getMemberName(name).c_str()); }
	void reflect(uint32_t &ref, const char *name) override { ml.reflect(ref, getMemberName(name).c_str()); }
	void reflect(float &ref, const char *name) override { ml.reflect(ref, getMemberName(name).c_str()); }
	void reflectAnyRef(kanyobjref &ref, int clfid, const char *name) override { ml.reflectAnyRef(ref, clfid, getMemberName(name).c_str()); }
	void reflect(Vector3 &ref, const char *name) override { ml.reflect(ref, getMemberName(name).c_str()); }
	void reflect(EventNode &ref, const char *name, CKObject *user) override { ml.reflect(ref, getMemberName(name).c_str(), user); }
	void reflect(std::string &ref, const char *name) override { ml.reflect(ref, getMemberName(name).c_str()); }
};

struct ReadingMemberListener : MemberListener {
	File *file; KEnvironment *kenv;
	ReadingMemberListener(File *file, KEnvironment *kenv) : file(file), kenv(kenv) {}
	void reflect(uint8_t &ref, const char *name) override;
	void reflect(uint16_t &ref, const char *name) override;
	void reflect(uint32_t &ref, const char *name) override;
	void reflect(float &ref, const char *name) override;
	void reflectAnyRef(kanyobjref &ref, int clfid, const char *name) override;
	void reflect(Vector3 &ref, const char *name) override;
	void reflect(EventNode &ref, const char *name, CKObject *user) override;
	void reflect(MarkerIndex &ref, const char *name);
	void reflectAnyPostRef(KAnyPostponedRef& postref, int clfid, const char* name) override;
	void reflect(std::string &ref, const char *name) override;
	void onReadImpl(void* ctx, FileAccessor fa) override;
};

struct WritingMemberListener : MemberListener {
	File *file; KEnvironment *kenv;
	WritingMemberListener(File *file, KEnvironment *kenv) : file(file), kenv(kenv) {}
	void reflect(uint8_t &ref, const char *name) override;
	void reflect(uint16_t &ref, const char *name) override;
	void reflect(uint32_t &ref, const char *name) override;
	void reflect(float &ref, const char *name) override;
	void reflectAnyRef(kanyobjref &ref, int clfid, const char *name) override;
	void reflect(Vector3 &ref, const char *name) override;
	void reflect(EventNode &ref, const char *name, CKObject *user) override;
	void reflect(MarkerIndex& ref, const char* name) override;
	void reflectAnyPostRef(KAnyPostponedRef& postref, int clfid, const char* name) override;
	void reflect(std::string &ref, const char *name) override;
	void onWriteImpl(void* ctx, FileAccessor fa) override;
};

// MemberListener that keeps track of the full names of reflected members
// (including parent structs and array indices). Just derive this class
// and call getFullName to get the full name of the previously
// reflected member. If names are not needed, derive directly from
// MemberListener instead to save some string operations.
struct NamedMemberListener : MemberListener {
	struct Scope {
		std::string fullName;
		int index = -1;
		const nlohmann::json* propJson = nullptr;
	};
	std::stack<Scope> scopeStack;

	std::vector<const nlohmann::json*> propertyLists;
	void setPropertyInfoList(Encyclopedia& encyclo, CKObject* object);
	const nlohmann::json* getPropertyJson(const char* name);

	std::string getTranslatedName(const char* name);
	std::string getFullName(const char* name);
	std::string getShortName(const char* name);

	virtual void enterArray(const char* name) override;

	void enterArray(const char* name, int startIndex) {
		enterArray(name);
		setNextIndex(startIndex);
	}

	virtual void setNextIndex(int index) override {
		scopeStack.top().index = index;
	}

	virtual void incrementIndex() override {
		scopeStack.top().index += 1;
	}

	virtual void leaveArray() override {
		scopeStack.pop();
	}

	virtual void enterStruct(const char* name) override;

	virtual void leaveStruct() override {
		scopeStack.pop();
	}
};

// Filter adapter for Member Listeners
// Derived class must contain the method:
//   bool cond(const char* name) : returns true to keep member, false to filter out
template <typename Der, typename Base> struct FilterMemberListener : Base {
	static_assert(std::is_base_of_v<MemberListener, Base>, "Base type is not a MemberListener");
	using Base::Base;
	bool cond(const char* name) { return ((Der*)this)->cond(name); }
	virtual void reflect(uint8_t& ref, const char* name) override { if (cond(name)) Base::reflect(ref, name); }
	virtual void reflect(uint16_t& ref, const char* name) override { if (cond(name)) Base::reflect(ref, name); }
	virtual void reflect(uint32_t& ref, const char* name) override { if (cond(name)) Base::reflect(ref, name); }
	virtual void reflect(int8_t& ref, const char* name) override { if (cond(name)) Base::reflect(ref, name); }
	virtual void reflect(int16_t& ref, const char* name) override { if (cond(name)) Base::reflect(ref, name); }
	virtual void reflect(int32_t& ref, const char* name) override { if (cond(name)) Base::reflect(ref, name); }
	virtual void reflect(float& ref, const char* name) override { if (cond(name)) Base::reflect(ref, name); }
	virtual void reflectAnyRef(kanyobjref& ref, int clfid, const char* name) override { if (cond(name)) Base::reflectAnyRef(ref, clfid, name); }
	virtual void reflect(Vector3& ref, const char* name) override { if (cond(name)) Base::reflect(ref, name); }
	virtual void reflect(Matrix& ref, const char* name) override { if (cond(name)) Base::reflect(ref, name); }
	virtual void reflect(EventNode& ref, const char* name, CKObject* user) override { if (cond(name)) Base::reflect(ref, name, user); }
	virtual void reflect(MarkerIndex& ref, const char* name) override { if (cond(name)) Base::reflect(ref, name); }
	virtual void reflect(std::string& ref, const char* name) override { if (cond(name)) Base::reflect(ref, name); }
	// TODO: missing overrides
};

// Member-reflected class
template <class T> struct CKMemberReflectable : T {
	virtual void virtualReflectMembers(MemberListener &r, KEnvironment *kenv = nullptr) = 0;
	void serialize(KEnvironment* kenv, File *file) override {
		WritingMemberListener r(file, kenv);
		this->virtualReflectMembers(r, kenv);
	}
	void deserialize(KEnvironment* kenv, File *file, size_t length) override {
		ReadingMemberListener r(file, kenv);
		this->virtualReflectMembers(r, kenv);
	}
};

template <class D, class T, int N> struct CKMRSubclass : CKSubclass<T, N> {
	void virtualReflectMembers(MemberListener &r, KEnvironment *kenv = nullptr) override {
		((D*)this)->reflectMembers2(r, kenv);
	}
};

// Weak reference to CKObject
template<typename T> struct KWeakRef {
	//static_assert(std::is_base_of<CKObject, T>::value, "T must be derived from CKObject");

	T* _ptr = nullptr;
	int _id = 0;

	T* get() const {
		if (!_ptr)
			return nullptr;
		auto it = CKObject::objIdMap.find(_ptr);
		if (it != CKObject::objIdMap.end() && it->second == _id)
			return _ptr;
		else
			return nullptr;
	}
	void set(T* ptr) { _ptr = ptr; if (ptr) _id = CKObject::objIdMap[ptr]; }

	T* operator->() const { return get(); }
	explicit operator bool() const { return get() != nullptr; }
	KWeakRef& operator=(const KWeakRef& kwr) = default;
	KWeakRef& operator=(KWeakRef&& kwr) = default;
	KWeakRef& operator=(T* ptr) { set(ptr); return *this; }

	bool operator==(const KWeakRef& kwr) const { if (T* g = get()) return g == kwr.get(); else return false; }
	bool operator==(CKObject* obj) const { return get() == obj; }

	KWeakRef() = default;
	KWeakRef(T* ptr) { set(ptr); }
	KWeakRef(const KWeakRef& kwr) = default;
	KWeakRef(KWeakRef&& kwr) = default;
};
