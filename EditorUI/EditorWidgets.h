#pragma once

#include "KObject.h"
#include "CKUtils.h"
#include "Events.h"

namespace EditorUI
{
	struct EditorInterface;

	void IGObjectSelector(EditorInterface& ui, const char* name, kanyobjref& ptr, uint32_t clfid = 0xFFFFFFFF);
	inline void IGObjectSelectorRef(EditorInterface& ui, const char* name, kanyobjref& ref) { IGObjectSelector(ui, name, ref, 0xFFFFFFFF); };
	inline void IGObjectSelectorRef(EditorInterface& ui, const char* name, kobjref<CKObject>& ref) { IGObjectSelector(ui, name, ref, 0xFFFFFFFF); };
	template<typename T> void IGObjectSelectorRef(EditorInterface& ui, const char* name, kobjref<T>& ref) { IGObjectSelector(ui, name, ref, T::FULL_ID); };

	void IGObjectSelector(EditorInterface& ui, const char* name, KAnyPostponedRef& postref, uint32_t clfid = 0xFFFFFFFF);
	inline void IGObjectSelectorRef(EditorInterface& ui, const char* name, KPostponedRef<CKObject>& postref) { IGObjectSelector(ui, name, postref, 0xFFFFFFFF); }
	template<typename T> void IGObjectSelectorRef(EditorInterface& ui, const char* name, KPostponedRef<T>& postref) { IGObjectSelector(ui, name, postref, T::FULL_ID); }

	using EventNodePayload = std::pair<EventNodeX2*, char[16]>;
	void IGEventSelector(EditorInterface& ui, const char* name, EventNode& ref);
	void IGEventSelector(EditorInterface& ui, const char* name, EventNodeX1& ref);
	void IGEventSelector(EditorInterface& ui, const char* name, EventNodeX2& ref);
	void IGMarkerSelector(EditorInterface& ui, const char* name, MarkerIndex& ref);
	void IGObjectDragDropSource(EditorInterface& ui, CKObject* obj);
	bool IGEventMessageSelector(EditorInterface& ui, const char* label, uint16_t& message, CKObject* kobj, bool isCallback = false);
	bool IGEventMessageSelector(EditorInterface& ui, const char* label, uint16_t& message, int fid, bool isCallback = false);
}