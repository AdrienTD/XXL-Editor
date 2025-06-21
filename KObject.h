#pragma once

#include <functional>
//#include <typeinfo>
#include <array>
#include <cassert>
#include <set>
#include <span>
#include <unordered_map>
#include <vector>

struct File;
struct KEnvironment;

struct CKObject {
	static std::unordered_map<CKObject*, int> refCounts;
	static std::unordered_map<CKObject*, int> objIdMap;
	//int refCount = 0;
	void addref() { refCounts[this]++; }
	void release() { refCounts[this]--; }
	int getRefCount() { return refCounts[this]; }
	virtual ~CKObject() { objIdMap[this]++; };
	virtual bool isSubclassOfID(uint32_t fid) = 0;
	virtual int getClassCategory() = 0;
	virtual int getClassID() = 0;
	virtual const char *getClassName() = 0;
	virtual std::span<const int> getClassHierarchy() = 0;
	virtual void deserialize(KEnvironment* kenv, File *file, size_t length);
	virtual void serialize(KEnvironment* kenv, File *file);
	virtual void onLevelLoaded(KEnvironment *kenv) {}
	virtual void onLevelLoaded2(KEnvironment *kenv) {}
	virtual void deserializeLvlSpecific(KEnvironment* kenv, File *file, size_t length) { CKObject::deserialize(kenv, file, length); }
	virtual void serializeLvlSpecific(KEnvironment* kenv, File *file) { CKObject::serialize(kenv, file); }
	virtual void resetLvlSpecific(KEnvironment *kenv) {}
	virtual void deserializeGlobal(KEnvironment* kenv, File* file, size_t length) { this->deserialize(kenv, file, length); }
	virtual void serializeGlobal(KEnvironment* kenv, File* file) { this->serialize(kenv, file); }
	virtual void init(KEnvironment *kenv) {}
	virtual int getAddendumVersion() { return 0; }
	virtual void deserializeAddendum(KEnvironment* kenv, File* file, int version) {}
	virtual void serializeAddendum(KEnvironment* kenv, File* file) {}
	virtual void onLevelSave(KEnvironment* kenv) {}

	bool isSubclassOfID(int clcat, int clid) { return isSubclassOfID(clcat | (clid << 6)); }
	template<typename T> bool isSubclassOf() { return isSubclassOfID(T::FULL_ID); }
	template<> static constexpr bool isSubclassOf<CKObject>() { return true; }
	uint32_t getClassFullID() { return getClassCategory() | (getClassID() << 6); }
	template<class T> T *cast() {
		assert(isSubclassOfID(T::FULL_ID) && "CKObject Cast Fail");
		return (T*)this;
	}
	template<class T> T *dyncast() {
		if (isSubclassOfID(T::FULL_ID))
			return (T*)this;
		else
			return nullptr;
	}
};

struct CKUnknown : CKObject {
	static constexpr int FULL_ID = -1;
	int clCategory, clId;
	std::vector<uint8_t> mem, lsMem;
	uint32_t offset, lvlSpecificOffset; // file offsets from where it was read, for info

	static std::set<std::pair<int, int>> hits;

	bool isSubclassOfID(uint32_t fid) override;
	int getClassCategory() override;
	int getClassID() override;
	const char *getClassName() override;
	std::span<const int> getClassHierarchy() override;
	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void deserializeLvlSpecific(KEnvironment* kenv, File *file, size_t length) override;
	void serializeLvlSpecific(KEnvironment* kenv, File *file) override;

	CKUnknown(int category, int id) : clCategory(category), clId(id) {
		hits.insert(std::make_pair(category, id));
	}
	CKUnknown(const CKUnknown &another);
};

struct KFactory {
	int fullid;
	std::span<const int> hierarchy;
	CKObject* (*create)();
	void (*copy)(const CKObject*, CKObject*);

	KFactory(uint32_t fullid, std::span<const int> hierarchy, CKObject* (*create)(), void (*copy)(const CKObject*, CKObject*)) :
		fullid(fullid), hierarchy(hierarchy), create(create), copy(copy) {}

	template<class T> static KFactory of() {
		return KFactory(T::FULL_ID, T::HIERARCHY,
			[]() -> CKObject* {return new T; },
			[](const CKObject* src, CKObject* dest) -> void { *(T*)dest = *(T*)src; });
	}
};

template<int T_CAT> struct CKCategory : CKObject {
	static constexpr int CATEGORY = T_CAT;
	static constexpr int FULL_ID = T_CAT;
	static constexpr std::integer_sequence<int, T_CAT> HIERARCHY_INT_SEQ;

	bool isSubclassOfID(uint32_t fid) override { return fid == CATEGORY; }
	int getClassCategory() override { return CATEGORY; }
	std::span<const int> getClassHierarchy() override { return std::span<const int>(&FULL_ID, 1); }
};

template<int N, int ... P> consteval auto ISPrepend(std::integer_sequence<int, P...> seq) { return std::integer_sequence<int, N, P...>(); }
template<int ... P> consteval auto ISToArray(std::integer_sequence<int, P...> seq) { constexpr std::array<int, seq.size()> arr = { P... }; return arr; }

template<class T, int T_ID, int T_CAT = T::CATEGORY> struct CKSubclass : T {
	static constexpr int CATEGORY = T_CAT;
	static constexpr int CLASS_ID = T_ID;
	static constexpr int FULL_ID = T_CAT | (T_ID << 6);
	static constexpr auto HIERARCHY_INT_SEQ = ISPrepend<FULL_ID>(T::HIERARCHY_INT_SEQ);
	inline static constinit auto HIERARCHY = ISToArray(HIERARCHY_INT_SEQ);
	
	bool isSubclassOfID(uint32_t fid) override {
		//printf("%i :: isSubclass(%i)\n", FULL_ID, fid);
		if (fid == FULL_ID)
			return true;
		return T::isSubclassOfID(fid);
	}
	
	int getClassCategory() override { return T_CAT; }
	int getClassID() override { return T_ID; }
	const char* getClassName() override { return typeid(*this).name() + 7; }
	std::span<const int> getClassHierarchy() override { return HIERARCHY; }
};

struct kanyobjref {
	CKObject *_pointer;
	//kanyobjref();
	void anyreset(CKObject *newpointer = nullptr) {
		if (_pointer)
			_pointer->release();
		_pointer = newpointer;
		if (_pointer)
			_pointer->addref();
	}
	CKObject* get() const { return _pointer; }
	explicit operator bool() const { return _pointer != nullptr; }
};

template<class T> struct kobjref : kanyobjref {
	//CKObject *_pointer;
	T *operator->() const { return (T*)_pointer; }
	T &operator*() const { return *(T*)_pointer; }
	void reset(T *newpointer = nullptr) {
		if (_pointer)
			_pointer->release();
		_pointer = (CKObject*)newpointer;
		if (_pointer)
			_pointer->addref();
	}
	T *get() const { return (T*)_pointer; }
	kobjref() { _pointer = nullptr; }
	kobjref(T *pointer)					{ _pointer = (CKObject*)pointer; if (_pointer) _pointer->addref(); }
	kobjref(const kobjref &another)		{ _pointer = another._pointer; if (_pointer) _pointer->addref(); }
	kobjref(kobjref &&another) noexcept	{ _pointer = another._pointer; another._pointer = nullptr; }
	kobjref & operator=(const kobjref &another) { reset((T*)another._pointer); return *this; }
	kobjref & operator=(kobjref &&another) noexcept {
		if (_pointer)
			_pointer->release();
		_pointer = another._pointer;
		another._pointer = nullptr;
		return *this;
	}
	//bool operator==(const kobjref &another) const { return _pointer == another._pointer; }
	//bool operator!=(const kobjref &another) const { return _pointer != another._pointer; }
	~kobjref() { if(_pointer) _pointer->release(); }
};

template<class T, class U> bool operator==(const kobjref<T> &a, const kobjref<U> &b) { return a._pointer == b._pointer; }
template<class T, class U> bool operator!=(const kobjref<T> &a, const kobjref<U> &b) { return a._pointer != b._pointer; }

//struct CKManager : CKCategory<0> {};
//struct CKService : CKCategory<1> {};
//struct CKHook : CKCategory<2> {};
//struct CKHookLife : CKCategory<3> {};
//struct CKGroup : CKCategory<4> {};
//struct CKGroupLife : CKCategory<5> {};
//struct CKComponent : CKCategory<6> {};
//struct CKCamera : CKCategory<7> {};
//struct CKCinematicBloc : CKCategory<8> {};
//struct CKDictionary : CKCategory<9> {};
//struct CKGeometry : CKCategory<10> {};
//struct CKNode : CKCategory<11> {};
//struct CKLogic : CKCategory<12> {};
//struct CKGraphical : CKCategory<13> {};
//struct CKError : CKCategory<14> {};