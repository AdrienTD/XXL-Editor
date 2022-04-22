#pragma once

#include "KObject.h"
#include "KEnvironment.h"
#include <string>
#include "File.h"
#include <stack>

struct MemberListener;
struct Vector3;
struct Matrix;
struct File;
struct EventNode;

struct KAnyPostponedRef {
	kobjref<CKObject> ref;
	uint32_t id = 0xFFFFFFFF;
	kuuid uuid;
	bool bound = false;

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
	void read(File* file) {
		id = file->readUint32();
		if (id == 0xFFFFFFFD)
			file->read(uuid.data(), 16);
	}
	void bind(KEnvironment* kenv, int sector) {
		KAnyPostponedRef::bind(kenv, sector);
		if (ref) {
			assert(ref->isSubclassOf<T>());
		}
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

	T *get() const { assert(bound); return (T*)ref.get(); }
	T *operator->() const { return get(); }

	//operator kobjref<T>&() { assert(bound); return ref; }
};

template<class U, class V> bool operator==(kobjref<U> &ref, KPostponedRef<V> &post) { return ref.get() == post.get(); }
template<class U, class V> bool operator==(KPostponedRef<V> &post, kobjref<U> &ref) { return ref.get() == post.get(); }

// Clonable class
template <class T> struct CKClonable : T {
	virtual CKClonable *clone(KEnvironment &kenv, int sector) = 0;
};

template <class D, class T, int N> struct CKClonableSubclass : CKSubclass<T, N> {
	CKClonableSubclass *clone(KEnvironment &kenv, int sector) override {
		D *clone = kenv.createObject<D>(sector);
		*clone = *(D*)this;
		return clone;
	}
};

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
	void reflect(std::string &ref, const char *name) override;
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
	void reflect(std::string &ref, const char *name) override;
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
	};
	std::stack<Scope> scopeStack;

	std::string getFullName(const char* name) {
		if (scopeStack.empty())
			return name;
		Scope& scope = scopeStack.top();
		if (scope.index == -1)
			return scope.fullName + '.' + name;
		else
			return scope.fullName + '[' + std::to_string(scope.index) + ']';
	}

	virtual void enterArray(const char* name) override {
		std::string fullName = getFullName(name);
		Scope& scope = scopeStack.emplace();
		scope.fullName = std::move(fullName);
		scope.index = 0;
	}
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

	virtual void enterStruct(const char* name) override {
		std::string fullName = getFullName(name);
		Scope& scope = scopeStack.emplace();
		scope.fullName = std::move(fullName);
		scope.index = -1;
	};

	virtual void leaveStruct() override {
		scopeStack.pop();
	}
};

// Member-reflected class
template <class T> struct CKMemberReflectable : CKClonable<T> {
	virtual void virtualReflectMembers(MemberListener &r, KEnvironment *kenv = nullptr) = 0;
};

template <class D, class T, int N> struct CKMRSubclass : CKClonableSubclass<D, T, N> {
	void reflectMembers2(MemberListener &r, KEnvironment *kenv) {
		((D*)this)->reflectMembers(r);
	}

	void serialize(KEnvironment* kenv, File *file) override {
		WritingMemberListener r(file, kenv);
		((D*)this)->reflectMembers2(r, kenv);
	}

	void deserialize(KEnvironment* kenv, File *file, size_t length) override {
		ReadingMemberListener r(file, kenv);
		((D*)this)->reflectMembers2(r, kenv);
	}
	void virtualReflectMembers(MemberListener &r, KEnvironment *kenv = nullptr) override {
		((D*)this)->reflectMembers2(r, kenv);
	}
};
