#pragma once

#include <vector>
#include <memory>
#include <map>
#include <algorithm>

struct KEnvironment;
struct File;

struct KLocalObject {
	virtual ~KLocalObject() {}
	virtual int getFullID() = 0;
	virtual void deserialize(KEnvironment *kenv, File *file, size_t length) = 0;
	virtual void serialize(KEnvironment *kenv, File *file) = 0;
	virtual KLocalObject *clone() = 0;
};

template<int CLCAT, int CLID> struct KLocalObjectSub : KLocalObject {
	static const int CATEGORY = CLCAT;
	static const int CLASS_ID = CLID;
	static const int FULL_ID = CLCAT | (CLID << 6);
	int getFullID() override { return FULL_ID; }
};

struct KLocalPack
{
	std::vector<std::unique_ptr<KLocalObject>> objects;
	std::map<int, KLocalObject*(*) ()> factories;
	void deserialize(KEnvironment *kenv, File *file);
	void serialize(KEnvironment *kenv, File *file);
	template <class T> void addFactory() { factories[T::FULL_ID] = []() -> KLocalObject* { return new T; }; }
	template <class T> T* get() {
		auto it = std::find_if(objects.begin(), objects.end(), [](const std::unique_ptr<KLocalObject> &ref) {return ref->getFullID() == T::FULL_ID; });
		if (it != objects.end())
			return dynamic_cast<T*>(it->get());
		return nullptr;
	}

	KLocalPack() = default;
	KLocalPack(const KLocalPack &pack) : factories(pack.factories) {
		for(auto &obj : pack.objects)
			this->objects.emplace_back(obj->clone());
	}
	KLocalPack(KLocalPack &&pack) = default;
	KLocalPack &operator=(KLocalPack &pack) {
		this->objects.clear();
		for (auto &obj : pack.objects)
			this->objects.emplace_back(obj->clone());
		factories = pack.factories;
		return *this;
	}
	KLocalPack &operator=(KLocalPack &&pack) = default;
};

struct KUnknownLocalObject : KLocalObject {
	int cls_fid = -1;
	void *memptr = nullptr; size_t memsize = 0;

	KUnknownLocalObject(int cls_fid) : cls_fid(cls_fid) {}

	int getFullID() override { return cls_fid; }
	void deserialize(KEnvironment *kenv, File *file, size_t length) override;
	void serialize(KEnvironment *kenv, File *file) override;
	KLocalObject *clone() override;
};