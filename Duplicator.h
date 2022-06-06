#pragma once

#include "CKUtils.h"
#include <functional>
#include <map>

struct KEnvironment;
struct EditorInterface;
struct CKObject;
struct CKHook;
struct CKGroup;
struct CKSceneNode;
namespace std::filesystem { class path; };

// Hook duplication that clones hooks and related objects by looking through every reference member of the hook.
// Works with some (hopefully most) types of hooks, though some others might need special treatment.
struct HookMemberDuplicator : MemberListener {
public:
	HookMemberDuplicator(KEnvironment& kenv, EditorInterface* ui) : kenv(kenv), ui(ui) {}
	void doClone(CKHook* hook);
	void doExport(CKHook* hook, const std::filesystem::path& path);
	void doImport(const std::filesystem::path& path, CKGroup* parent);

private:
	KEnvironment& kenv;
	KEnvironment* srcEnv; KEnvironment* destEnv;
	EditorInterface* ui;
	std::map<CKObject*, CKObject*> cloneMap;
	std::function<CKObject* (CKObject*, int)> cloneFunction;

	template <typename T> T* cloneWrap(T* obj, int sector = -1) {
		return (T*)cloneFunction(obj, sector);
	}

	CKSceneNode* cloneNode(CKSceneNode* original, bool recursive);
	static CKGroup* findGroup(CKHook* hook, CKGroup* root);

	virtual void reflect(uint8_t& ref, const char* name);
	virtual void reflect(uint16_t& ref, const char* name);
	virtual void reflect(uint32_t& ref, const char* name);
	virtual void reflect(float& ref, const char* name);
	virtual void reflectAnyRef(kanyobjref& ref, int clfid, const char* name);
	virtual void reflect(Vector3& ref, const char* name);
	virtual void reflect(EventNode& ref, const char* name, CKObject* user);
	virtual void reflect(std::string& ref, const char* name);

	CKHook* doCommon(CKHook* hook);
	CKHook* doTransfer(CKHook* hook, KEnvironment* srcEnv, KEnvironment* destEnv);
	
	static void saveKFab(KEnvironment& kfab, CKObject* mainObj, const std::filesystem::path& path);
	static CKObject* loadKFab(KEnvironment& kfab, const std::filesystem::path& path);
};