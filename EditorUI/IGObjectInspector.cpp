#include "IGObjectInspector.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "ImGuiMemberListener.h"
#include "DictionaryEditors.h"

#include "KEnvironment.h"
#include "CoreClasses/CKManager.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKHook.h"
#include "CoreClasses/CKGroup.h"
#include "CoreClasses/CKComponent.h"
#include "CoreClasses/CKCamera.h"
#include "CoreClasses/CKCinematicNode.h"
#include "CoreClasses/CKDictionary.h"
#include "CoreClasses/CKLogic.h"
#include "CoreClasses/CKGraphical.h"

#include <imgui/imgui.h>

namespace
{
	template<typename Func, typename First, typename ... Rest> void EI_ReflectAnyReflectableObject(const Func& f, CKObject* obj)
	{
		if (auto* s = obj->dyncast<First>())
			f(s);
		else if constexpr (sizeof...(Rest) > 0)
			EI_ReflectAnyReflectableObject<Func, Rest...>(f, obj);
	}
}

void EditorUI::IGObjectInspector(EditorInterface& ui)
{
	auto& kenv = ui.kenv;
	kobjref<CKObject> comboObject = ui.selectedInspectorObjectRef.get();
	IGObjectSelectorRef(ui, "Object", comboObject);
	if (comboObject.get() != ui.selectedInspectorObjectRef.get())
		ui.selectedInspectorObjectRef = comboObject.get();
	if (CKObject* obj = ui.selectedInspectorObjectRef.get()) {
		IGObjectNameInput("Name", obj, kenv);
		ImGui::Separator();
		ImGui::BeginChild("ObjReflection");
		ImGuiMemberListener ml{ kenv, ui };
		ml.setPropertyInfoList(ui.g_encyclo, obj);
		auto f = [&](auto s) {s->virtualReflectMembers(ml, &kenv); };
		EI_ReflectAnyReflectableObject<decltype(f),
			CKReflectableManager, CKReflectableService, CKHook, CKGroup, CKReflectableComponent,
			CKCameraBase, CKCinematicNode, CKReflectableLogic, CKReflectableGraphical>(f, obj);
		if (auto* s = obj->dyncast<CKReflectableGameDef>()) {
			s->reflectLevel(ml, &kenv);
		}
		if (auto* animDict = obj->dyncast<CAnimationDictionary>()) {
			AnimDictEditor(ui, animDict, false);
		}
		if (auto* sndDict = obj->dyncast<CKSoundDictionaryID>()) {
			SoundDictIDEditor(ui, sndDict, false);
		}
		ImGui::EndChild();
	}
}
