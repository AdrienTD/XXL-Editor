#pragma once

#include "KObject.h"
#include "KEnvironment.h"
#include <string>

struct MemberListener;
struct Vector3;
struct File;

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

	struct MinusFID {
		static const int FULL_ID = -1;
	};
	template <class T> void reflect(kobjref<T> &ref, const char *name) {
		int fid = std::conditional<std::is_same<T, CKObject>::value, MinusFID, T>::type::FULL_ID;
		reflectAnyRef(ref, fid, name);
	};
	template <class T> void reflectContainer(T &ref, const char *name) {
		char txt[32];
		int i = 0;
		for (auto &elem : ref) {
			sprintf_s(txt, "%s[%u]", name, i++);
			reflect(elem, txt);
		}
	}
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
};

// Member-reflected class
template <class T> struct CKMemberReflectable : CKClonable<T> {
	virtual void virtualReflectMembers(MemberListener &r) = 0;
};

template <class D, class T, int N> struct CKMRSubclass : CKClonableSubclass<D, T, N> {
	void serialize(KEnvironment* kenv, File *file) override {
		WritingMemberListener r(file, kenv);
		((D*)this)->reflectMembers(r);
	}

	void deserialize(KEnvironment* kenv, File *file, size_t length) override {
		ReadingMemberListener r(file, kenv);
		((D*)this)->reflectMembers(r);
	}
	void virtualReflectMembers(MemberListener &r) override {
		((D*)this)->reflectMembers(r);
	}
};
