#include "EditorWidgets.h"
#include "EditorInterface.h"
#include "CoreClasses/CKService.h"
#include <imgui/imgui.h>
#include "GuiUtils.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>

using namespace GuiUtils;

void EditorUI::IGObjectSelector(EditorUI::EditorInterface& ui, const char* name, kanyobjref& ptr, uint32_t clfid)
{
	auto& kenv = ui.kenv;
	auto className = [](CKObject* obj) -> const char* {
		static char unkbuf[32];
		if (dynamic_cast<CKUnknown*>(obj)) {
			sprintf_s(unkbuf, "(%i, %i)", obj->getClassCategory(), obj->getClassID());
			return unkbuf;
		}
		else
			return obj->getClassName();
		};
	char tbuf[128] = "(null)";
	if (CKObject* obj = ptr._pointer)
		_snprintf_s(tbuf, _TRUNCATE, "%s : %s (%p)", className(obj), kenv.getObjectName(obj), obj);
	if (ImGui::BeginCombo(name, tbuf, 0)) {
		if (ImGui::Selectable("(null)", ptr._pointer == nullptr))
			ptr.anyreset();
		for (uint32_t clcatnum = 0; clcatnum < 15; clcatnum++) {
			if (clfid != 0xFFFFFFFF && (clfid & 63) != clcatnum)
				continue;
			auto& clcat = kenv.levelObjects.categories[clcatnum];
			for (uint32_t clid = 0; clid < clcat.type.size(); clid++) {
				auto& cl = clcat.type[clid];
				for (CKObject* eo : cl.objects) {
					if (clfid != 0xFFFFFFFF && !eo->isSubclassOfID(clfid))
						continue;
					ImGui::PushID(eo);
					if (ImGui::Selectable("##objsel", eo == ptr._pointer)) {
						ptr.anyreset(eo);
					}
					if (ImGui::IsItemVisible()) {
						ImGui::SameLine(0.0f, 0.0f);
						ImGui::Text("%s : %s (%p)", className(eo), kenv.getObjectName(eo), eo);
					}
					ImGui::PopID();
				}
			}
		}
		ImGui::EndCombo();
	}
	if (ptr) {
		IGObjectDragDropSource(ui, ptr._pointer);
	}
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::GetDragDropPayload()) {
			if (payload->IsDataType("CKObject")) {
				CKObject* obj = *(CKObject**)payload->Data;
				if (clfid == 0xFFFFFFFF || obj->isSubclassOfID(clfid))
					if (const ImGuiPayload* acceptedPayload = ImGui::AcceptDragDropPayload("CKObject"))
						ptr.anyreset(*(CKObject**)payload->Data);
			}
		}
		ImGui::EndDragDropTarget();
	}
}

void EditorUI::IGObjectDragDropSource(EditorUI::EditorInterface& ui, CKObject* obj)
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		ImGui::SetDragDropPayload("CKObject", &obj, sizeof(obj));
		ImGui::Text("%p : %i %i %s : %s", obj, obj->getClassCategory(), obj->getClassID(), obj->getClassName(), ui.kenv.getObjectName(obj));
		ImGui::EndDragDropSource();
	}
}

void EditorUI::IGObjectSelector(EditorUI::EditorInterface& ui, const char* name, KAnyPostponedRef& postref, uint32_t clfid)
{
	if (postref.bound)
		IGObjectSelector(ui, name, postref.ref, clfid);
	else {
		uint32_t& tuple = postref.id;
		int igtup[3] = { tuple & 63, (tuple >> 6) & 2047, tuple >> 17 };
		if (ImGui::InputInt3(name, igtup)) {
			tuple = (igtup[0] & 63) | ((igtup[1] & 2047) << 6) | ((igtup[2] & 32767) << 17);
		}
	}
}

void EditorUI::IGEventSelector(EditorUI::EditorInterface& ui, const char* name, EventNode& ref) {
	if (ui.kenv.version < KEnvironment::KVERSION_XXL2)
		IGEventSelector(ui, name, ref.enx1);
	else
		IGEventSelector(ui, name, ref.enx2);
}

void EditorUI::IGEventSelector(EditorUI::EditorInterface& ui, const char* name, EventNodeX1& ref) {
	CKSrvEvent* srvEvent = ui.kenv.levelObjects.getFirst<CKSrvEvent>();
	const char* preview = "?";
	if (ref.seqIndex == -1)
		preview = "(none)";
	else {
		auto& seqids = srvEvent->evtSeqIDs;
		auto it = std::find(seqids.begin(), seqids.end(), ref.seqIndex);
		if (it != seqids.end()) {
			int evtActualIndex = (int)(it - seqids.begin());
			if (evtActualIndex >= 0 && evtActualIndex < srvEvent->sequences.size())
				preview = srvEvent->evtSeqNames[evtActualIndex].c_str();
		}
	}

	ImGui::PushID(name);
	ImGui::BeginGroup();
	float itemwidth = ImGui::CalcItemWidth();
	ImGui::SetNextItemWidth(itemwidth - 32.0f - 2 * ImGui::GetStyle().ItemInnerSpacing.x - ImGui::GetFrameHeight());
	if (ImGui::BeginCombo("##HkEventBox", preview)) {
		if (ImGui::Selectable("(none)", ref.seqIndex == -1))
			ref.seqIndex = -1;
		for (int i = 0; i < srvEvent->sequences.size(); ++i) {
			ImGui::PushID(i);
			if (ImGui::Selectable(srvEvent->evtSeqNames[i].c_str(), ref.seqIndex == srvEvent->evtSeqIDs[i]))
				ref.seqIndex = srvEvent->evtSeqIDs[i];
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::SetNextItemWidth(32.0f);
	if (ImGui::InputScalar("##EventBitNode", ImGuiDataType_U8, &ref.bit))
		ref.bit &= 7;
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	if (ImGui::ArrowButton("HkSelectEvent", ImGuiDir_Right)) {
		auto& seqids = srvEvent->evtSeqIDs;
		auto it = std::find(seqids.begin(), seqids.end(), ref.seqIndex);
		if (it != seqids.end()) {
			ui.selectedEventSequence = (int)(it - seqids.begin());
			ui.wndShowEvents = true;
		}
		else
			ImGui::OpenPopup("EventNodeNotFound");
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Select event sequence");
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text(name);
	ImGui::EndGroup();
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		EventNodeX1* data = &ref;
		ImGui::SetDragDropPayload("EventNodeX1", &data, sizeof(data));
		ImGui::Text("Event node");
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EventSeq")) {
			ref.seqIndex = *(uint16_t*)payload->Data;
			ref.bit = 0;
		}
		ImGui::EndDragDropTarget();
	}
	if (ImGui::BeginPopup("EventNodeNotFound")) {
		ImGui::Text("Event sequence of ID %i no longer exists.", ref.seqIndex);
		ImGui::EndPopup();
	}
	ImGui::PopID();
}

void EditorUI::IGEventSelector(EditorUI::EditorInterface& ui, const char* name, EventNodeX2& ref) {
	ref.clean();
	ImGui::LabelText(name, "%zi CmpDatas", ref.datas.size());
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		EventNodePayload data;
		data.first = &ref;
		strncpy_s(data.second, name, sizeof(data.second));
		ImGui::SetDragDropPayload("EventNodeX2", &data, sizeof(data));
		ImGui::Text("Event node %s", name);
		ImGui::EndDragDropSource();
	}
}

void EditorUI::IGMarkerSelector(EditorUI::EditorInterface& ui, const char* name, MarkerIndex& ref)
{
	auto& kenv = ui.kenv;
	ImGui::PushID(name);
	float itemwidth = ImGui::CalcItemWidth();
	ImGui::SetNextItemWidth(itemwidth - ImGui::GetStyle().ItemInnerSpacing.x - ImGui::GetFrameHeight());
	if (kenv.version == KEnvironment::KVERSION_XXL1)
		ImGui::InputInt("##MarkerInput", &ref.index);
	else if (kenv.version >= KEnvironment::KVERSION_XXL2)
		ImGui::InputScalarN("##MarkerInput", ImGuiDataType_U8, &ref.index, 4);
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	if (ImGui::ArrowButton("SelectMarker", ImGuiDir_Right)) {
		if (kenv.version == KEnvironment::KVERSION_XXL1) {
			CKSrvMarker* srvMarker = kenv.levelObjects.getFirst<CKSrvMarker>();
			if (srvMarker && !srvMarker->lists.empty()) {
				auto& list = srvMarker->lists.front();
				if (ref.index >= 0 && ref.index < (int)list.size()) {
					ui.selectedMarkerIndex = ref.index;
					ui.wndShowMarkers = true;
				}
			}
		}
		else if (kenv.version >= KEnvironment::KVERSION_XXL2) {
			ui.selBeaconSector = ref.index & 255;
			ui.selBeaconKluster = (ref.index >> 8) & 255;
			ui.selBeaconBing = (ref.index >> 16) & 255;
			ui.selBeaconIndex = (ref.index >> 24) & 255;
			ui.wndShowBeacons = true;
		}
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Select marker");
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::TextUnformatted(name);
	ImGui::PopID();
}

bool EditorUI::IGEventMessageSelector(EditorUI::EditorInterface& ui, const char* label, uint16_t& message, CKObject* kobj, bool isCallback)
{
	return IGEventMessageSelector(ui, label, message, kobj ? (int)kobj->getClassFullID() : -1, isCallback);
}

bool EditorUI::IGEventMessageSelector(EditorUI::EditorInterface& ui, const char* label, uint16_t& message, int fid, bool isCallback)
{
	auto& kenv = ui.kenv;
	auto& g_encyclo = ui.g_encyclo;
	static const std::string msgActionCallbacksSetName = "Squad MsgAction Callbacks";
	bool modified = false;

	const nlohmann::json* eventJson = nullptr;
	if (kenv.factories.contains(fid))
		eventJson = g_encyclo.getEventJson(kenv.factories.at(fid).hierarchy, message);
	else
		eventJson = g_encyclo.getEventJson(fid, message);

	std::string eventDesc = g_encyclo.getEventName(eventJson, message);
	bool hasIndex = eventJson && eventJson->at("id").get_ref<const std::string&>().size() == 9;
	float original_x = ImGui::GetCursorPosX();
	float width = ImGui::CalcItemWidth();
	ImGui::SetNextItemWidth(width - (hasIndex ? 32.0f + ImGui::GetStyle().ItemSpacing.x : 0.0f));
	if (ImGui::BeginCombo("##EventCombo", eventDesc.c_str())) {
		modified |= ImGui::InputScalar("##EventID", ImGuiDataType_U16, &message, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);

		auto lookAtList = [&message, &modified](const nlohmann::json& evlist) {
			for (auto& ev : evlist) {
				auto& strId = ev.at("id").get_ref<const std::string&>();
				auto& strName = ev.at("name").get_ref<const std::string&>();
				std::string desc = strId + ": " + strName;
				auto [idStart, idEnd] = Encyclopedia::decodeRange(strId);
				if (ImGui::Selectable(desc.c_str(), idStart <= message && message <= idEnd)) {
					message = idStart;
					modified = true;
				}
			}
			};

		if (isCallback) {
			lookAtList(g_encyclo.eventSets.at(msgActionCallbacksSetName).at("events"));
		}
		else if (fid != -1) {
			std::span<const int> fids = { &fid, 1 };
			if (kenv.factories.contains(fid)) {
				fids = kenv.factories.at(fid).hierarchy;
			}
			for (const int fid : fids) {
				if (auto cit = g_encyclo.kclasses.find(fid); cit != g_encyclo.kclasses.end()) {
					if (auto itEvents = cit->second.find("events"); itEvents != cit->second.end())
						lookAtList(itEvents.value());
					if (auto itIncludes = cit->second.find("includeSets"); itIncludes != cit->second.end()) {
						for (auto& incName : itIncludes.value()) {
							auto& strIncName = incName.get_ref<const std::string&>();
							if (strIncName == msgActionCallbacksSetName)
								continue;
							auto& incSet = g_encyclo.eventSets.at(strIncName);
							lookAtList(incSet.at("events"));
						}
					}
				}
			}
		}
		ImGui::EndCombo();
	}
	if (hasIndex) {
		ImGui::SameLine();
		ImGui::SetNextItemWidth(32.0f);
		auto [idStart, idEnd] = Encyclopedia::decodeRange(eventJson->at("id").get_ref<const std::string&>());
		int index = (int)message - idStart;
		if (ImGui::InputInt("##EventIndex", &index, 0, 0)) {
			int newIndex = std::clamp(idStart + index, idStart, idEnd);
			message = (uint16_t)newIndex;
			modified = true;
		}
	}
	ImGui::SameLine(original_x);
	ImGui::LabelText(label, "");
	return modified;
}

void EditorUI::IGObjectNameInput(const char* label, CKObject* obj, KEnvironment& kenv)
{
	std::string* pstr = nullptr;
	auto it = kenv.globalObjNames.dict.find(obj);
	if (it != kenv.globalObjNames.dict.end())
		pstr = &it->second.name;
	else {
		it = kenv.levelObjNames.dict.find(obj);
		if (it != kenv.levelObjNames.dict.end())
			pstr = &it->second.name;
		else {
			for (auto& str : kenv.sectorObjNames) {
				it = str.dict.find(obj);
				if (it != str.dict.end())
					pstr = &it->second.name;
			}
		}
	}
	if (pstr) {
		ImGui::InputText(label, pstr->data(), pstr->capacity() + 1, ImGuiInputTextFlags_CallbackResize, IGStdStringInputCallback, pstr);
	}
	else {
		char test[2] = { 0,0 };
		void* user[2] = { obj, &kenv };
		ImGui::InputText(label, test, 1, ImGuiInputTextFlags_CallbackResize,
			[](ImGuiInputTextCallbackData* data) -> int {
				if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
					void** user = (void**)data->UserData;
					CKObject* obj = (CKObject*)user[0];
					KEnvironment* kenv = (KEnvironment*)user[1];
					auto& info = kenv->makeObjInfo(obj);
					info.name.resize(data->BufTextLen);
					data->Buf = info.name.data();
				}
				return 0;
			}, user);
	}
}

void EditorUI::IGStringInput(const char* label, std::string& str)
{
	ImGui::InputText(label, str.data(), str.capacity() + 1, ImGuiInputTextFlags_CallbackResize, IGStdStringInputCallback, &str);
}

void EditorUI::IGLink(const char* text, const wchar_t* url, Window* window)
{
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 box = ImGui::CalcTextSize(text);
	uint32_t color = 0xFFFA870F;
	if (ImGui::InvisibleButton(text, box)) {
		ShellExecuteW(window ? (HWND)window->getNativeWindow() : nullptr, NULL, url, NULL, NULL, SW_SHOWNORMAL);
	}
	if (ImGui::IsItemHovered()) {
		color = 0xFFFFC74F;
		ImGui::SetTooltip("%S", url);
	}

	float ly = pos.y + box.y;
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddText(pos, color, text);
	drawList->AddLine(ImVec2(pos.x, ly), ImVec2(pos.x + box.x, ly), color);
}

bool EditorUI::IGU32Color(const char* name, uint32_t& color) {
	ImVec4 cf = ImGui::ColorConvertU32ToFloat4(color);
	if (ImGui::ColorEdit4(name, &cf.x)) {
		color = ImGui::ColorConvertFloat4ToU32(cf);
		return true;
	}
	return false;
}
