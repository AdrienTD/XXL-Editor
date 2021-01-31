#pragma once

#include "KObject.h"
#include "KEnvironment.h"
#include <string>
#include "File.h"

struct MemberListener;
struct Vector3;
struct Matrix;
struct File;
struct EventNode;

// Reference that stores the serialized form of the reference,
// and only deserializes after a call to bind().
// Useful in cases where you have to read a reference to an object from STR,
// but you know the exact sector number only later in the level loading process.
template <class T> struct KPostponedRef {
	kobjref<T> ref;
	uint32_t id = 0xFFFFFFFF;
	bool bound = false;

	void read(File *file) { id = file->readUint32(); assert(id != 0xFFFFFFFD); }
	void bind(KEnvironment *kenv, int sector) { ref = kenv->getObjRef<T>(id, sector); bound = true; }
	void write(KEnvironment *kenv, File *file) const { if (bound) kenv->writeObjRef<T>(file, ref); else file->writeUint32(id); }

	T *get() const { assert(bound); return ref.get(); }
	T *operator->() const { return get(); }

	operator kobjref<T>&() { assert(bound); return ref; }
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

struct MemberListener {
	virtual void reflect(uint8_t &ref, const char *name) = 0;
	virtual void reflect(uint16_t &ref, const char *name) = 0;
	virtual void reflect(uint32_t &ref, const char *name) = 0;
	virtual void reflect(float &ref, const char *name) = 0;
	virtual void reflectAnyRef(kanyobjref &ref, int clfid, const char *name) = 0;
	virtual void reflect(Vector3 &ref, const char *name) = 0;
	virtual void reflect(EventNode &ref, const char *name, CKObject *user /*= nullptr*/) = 0;
	virtual void reflectPostRefTuple(uint32_t &tuple, const char *name) { reflect(tuple, name); }
	virtual void reflect(std::string &ref, const char *name) = 0;

	struct MinusFID {
		static const int FULL_ID = -1;
	};
	template <class T> void reflect(kobjref<T> &ref, const char *name) {
		int fid = std::conditional<std::is_same<T, CKObject>::value, MinusFID, T>::type::FULL_ID;
		reflectAnyRef(ref, fid, name);
	};
	template <class T> void reflect(KPostponedRef<T> &ref, const char *name) {
		if (ref.bound)
			reflect(ref.ref, name);
		else
			reflectPostRefTuple(ref.id, name);
	}

	template <class T> void reflectContainer(T &ref, const char *name) {
		char txt[32];
		int i = 0;
		for (auto &elem : ref) {
			sprintf_s(txt, "%s[%u]", name, i++);
			reflect(elem, txt);
		}
	}
	template <class T, size_t N> void reflect(std::array<T, N> &ref, const char *name) { reflectContainer(ref, name); }
	template <class T> void reflect(std::vector<T> &ref, const char *name) { reflectContainer(ref, name); }

	void reflect(Matrix &ref, const char *name);

	template <class T> void reflect(T &ref, const char *name) {
		static_assert(std::is_class<T>::value, "cannot be reflected");
		//ref.reflectMembers(*this);
		StructMemberListener sml(*this, name);
		ref.reflectMembers(sml);
	}
	template <class SizeInt, class T> void reflectSize(T &container, const char *name) {
		SizeInt siz, newsiz;
		siz = newsiz = (SizeInt)container.size();
		reflect(newsiz, name);
		if (newsiz != siz)
			container.resize(newsiz);
	}
};

struct StructMemberListener : MemberListener {
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
