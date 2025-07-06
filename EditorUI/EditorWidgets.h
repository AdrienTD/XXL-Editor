#pragma once

#include "KObject.h"
#include "CKUtils.h"
#include "Events.h"

struct Window;

namespace EditorUI
{
	struct EditorInterface;

	bool IGObjectSelector(EditorInterface& ui, const char* name, kanyobjref& ptr, uint32_t clfid = 0xFFFFFFFF);
	inline bool IGObjectSelectorRef(EditorInterface& ui, const char* name, kanyobjref& ref) { return IGObjectSelector(ui, name, ref, 0xFFFFFFFF); };
	inline bool IGObjectSelectorRef(EditorInterface& ui, const char* name, kobjref<CKObject>& ref) { return IGObjectSelector(ui, name, ref, 0xFFFFFFFF); };
	template<typename T> bool IGObjectSelectorRef(EditorInterface& ui, const char* name, kobjref<T>& ref) { return IGObjectSelector(ui, name, ref, T::FULL_ID); };

	bool IGObjectSelector(EditorInterface& ui, const char* name, KAnyPostponedRef& postref, uint32_t clfid = 0xFFFFFFFF);
	inline bool IGObjectSelectorRef(EditorInterface& ui, const char* name, KPostponedRef<CKObject>& postref) { return IGObjectSelector(ui, name, postref, 0xFFFFFFFF); }
	template<typename T> bool IGObjectSelectorRef(EditorInterface& ui, const char* name, KPostponedRef<T>& postref) { return IGObjectSelector(ui, name, postref, T::FULL_ID); }

	using EventNodePayload = std::pair<EventNodeX2*, char[64]>;
	void IGEventSelector(EditorInterface& ui, const char* name, EventNode& ref);
	void IGEventSelector(EditorInterface& ui, const char* name, EventNodeX1& ref);
	void IGEventSelector(EditorInterface& ui, const char* name, EventNodeX2& ref);
	void IGMarkerSelector(EditorInterface& ui, const char* name, MarkerIndex& ref);
	void IGObjectDragDropSource(EditorInterface& ui, CKObject* obj);
	bool IGEventMessageSelector(EditorInterface& ui, const char* label, uint16_t& message, CKObject* kobj, bool isCallback = false);
	bool IGEventMessageSelector(EditorInterface& ui, const char* label, uint16_t& message, int fid, bool isCallback = false);

	void IGObjectNameInput(const char* label, CKObject* obj, KEnvironment& kenv);
	void IGStringInput(const char* label, std::string& str);
	bool IGU32Color(const char* name, uint32_t& color);
}