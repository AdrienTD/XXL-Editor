#include "ImGuiMemberListener.h"

#include <imgui/imgui.h>
#include "EditorWidgets.h"
#include "vecmat.h"
#include "CoreClasses/CKDictionary.h"
#include "DictionaryEditors.h"
#include "GuiUtils.h"
#include <nlohmann/json.hpp>
#include "PropFlagsEditor.h"

namespace
{
	enum class SpecialEditorResult {
		Unhandled = 0,
		Unmodified,
		Modified
	};

	SpecialEditorResult specialEditor32(EditorUI::ImGuiMemberListener& igml, const char* name, uint32_t& value)
	{
		const auto* json = igml.getPropertyJson(name);
		if (!json || !json->is_object()) {
			return SpecialEditorResult::Unhandled;
		}
		auto propType = json->value<std::string>("type", {});
		
		if (propType == "bool") {
			bool checked = value != 0;
			bool modified = ImGui::Checkbox(igml.getShortName(name).c_str(), &checked);
			if (modified) {
				value = checked ? 1 : 0;
				return SpecialEditorResult::Modified;
			}
			return SpecialEditorResult::Unmodified;
		}
		if (propType == "color") {
			ImVec4 components = ImGui::ColorConvertU32ToFloat4(value);
			bool modified = ImGui::ColorEdit4(igml.getShortName(name).c_str(), &components.x);
			if (modified) {
				value = ImGui::ColorConvertFloat4ToU32(components);
				return SpecialEditorResult::Modified;
			}
			return SpecialEditorResult::Unmodified;
		}

		if (auto itEnums = json->find("enums"); itEnums != json->end()) {
			const char* preview;
			std::string givenEnumKey = std::to_string(value);
			auto itGivenEnum = itEnums->find(givenEnumKey);
			if (itGivenEnum == itEnums->end()) {
				givenEnumKey = "Unknown enum " + std::move(givenEnumKey);
				preview = givenEnumKey.c_str();
			}
			else {
				preview = itGivenEnum->get_ref<const std::string&>().c_str();
			}

			bool modified = false;
			if (ImGui::BeginCombo(igml.getShortName(name).c_str(), preview)) {
				if (ImGui::InputScalar("Value", ImGuiDataType_U32, &value)) {
					modified = true;
				}
				for (auto& [availKey, availValue] : itEnums->items()) {
					uint32_t enumNumber = 0;
					auto conv = std::from_chars(availKey.data(), availKey.data() + availKey.size(), enumNumber);
					if (conv.ec == std::errc{}) {
						if (ImGui::Selectable(availValue.get_ref<const std::string&>().c_str(), enumNumber == value)) {
							value = enumNumber;
							modified = true;
						}
					}
				}
				ImGui::EndCombo();
			}

			return modified ? SpecialEditorResult::Modified : SpecialEditorResult::Unmodified;
		}

		return SpecialEditorResult::Unhandled;
	}

	template<std::integral T>
	bool specialEditor(EditorUI::ImGuiMemberListener& igml, const char* name, T& value)
	{
		uint32_t value32 = static_cast<uint32_t>(value);
		auto result = specialEditor32(igml, name, value32);
		if (result == SpecialEditorResult::Modified) {
			value = static_cast<T>(value32);
		}
		return result != SpecialEditorResult::Unhandled;
	}

	template<std::integral T>
	void integerEditor(EditorUI::ImGuiMemberListener& igml, const char* name, T& value, ImGuiDataType dataType)
	{
		bool handled = specialEditor(igml, name, value);
		if (handled)
			return;

		if ((int)igml.currentFlags & (int)MemberListener::MemberFlags::MF_SIZE_VALUE) {
			T modifiedValue = value;
			bool modified = ImGui::InputScalar(igml.getShortName(name).c_str(), dataType, &modifiedValue);
			if (modified && (int32_t)modifiedValue >= 0 && (int32_t)modifiedValue < 32768 && ImGui::IsItemDeactivatedAfterEdit()) {
				value = modifiedValue;
			}
		}
		else {
			ImGui::InputScalar(igml.getShortName(name).c_str(), dataType, &value);
		}

		igml.flagsEditor(name, value);
	}
}

bool EditorUI::ImGuiMemberListener::icon(const char* label, const char* desc)
{
	if (!scopeExpanded.empty() && scopeExpanded.top() == false)
		return false;
	bool hidden = (int)currentFlags & (int)MemberFlags::MF_EDITOR_HIDDEN;
	if (hidden)
		return false;
	ImGui::AlignTextToFramePadding();
	ImGui::TextColored(ImVec4(0, 1, 1, 1), label);
	if (desc && ImGui::IsItemHovered())
		ImGui::SetTooltip(desc);
	ImGui::SameLine();
	return true;
}

template<std::integral T>
void EditorUI::ImGuiMemberListener::flagsEditor(const char* name, T& value)
{
	if (const auto* jsProp = getPropertyJson(name)) {
		if (jsProp->is_object()) {
			if (auto itBitflags = jsProp->find("bitFlags"); itBitflags != jsProp->end()) {
				unsigned int flags = static_cast<unsigned int>(value);
				ImGui::Indent();
				if (PropFlagsEditor(flags, itBitflags.value())) {
					value = static_cast<T>(flags);
				}
				ImGui::Unindent();
			}
		}
	}
}

void EditorUI::ImGuiMemberListener::reflect(uint8_t& ref, const char* name)
{
	if (icon(" 8", "Unsigned 8-bit integer")) {
		integerEditor(*this, name, ref, ImGuiDataType_U8);
	}
}

void EditorUI::ImGuiMemberListener::reflect(uint16_t& ref, const char* name)
{
	if (icon("16", "Unsigned 16-bit integer")) {
		integerEditor(*this, name, ref, ImGuiDataType_U16);
	}
}

void EditorUI::ImGuiMemberListener::reflect(uint32_t& ref, const char* name)
{
	if (icon("32", "Unsigned 32-bit integer")) {
		integerEditor(*this, name, ref, ImGuiDataType_U32);
	}
}

void EditorUI::ImGuiMemberListener::reflect(int8_t& ref, const char* name) {
	if (icon(" 8", "Signed 8-bit integer")) {
		integerEditor(*this, name, ref, ImGuiDataType_S8);
	}
}

void EditorUI::ImGuiMemberListener::reflect(int16_t& ref, const char* name) {
	if (icon("16", "Signed 16-bit integer")) {
		integerEditor(*this, name, ref, ImGuiDataType_S16);
	}
}

void EditorUI::ImGuiMemberListener::reflect(int32_t& ref, const char* name) {
	if (icon("32", "Signed 32-bit integer")) {
		integerEditor(*this, name, ref, ImGuiDataType_S32);
	}
}

void EditorUI::ImGuiMemberListener::reflect(float& ref, const char* name) {
	if (icon("Fl", "IEEE 754 Single floating-point number"))
		ImGui::InputScalar(getShortName(name).c_str(), ImGuiDataType_Float, &ref);
}

void EditorUI::ImGuiMemberListener::reflectAnyRef(kanyobjref& ref, int clfid, const char* name) {
	if (icon("Rf", "Object reference")) {
		EditorUI::IGObjectSelector(ui, getShortName(name).c_str(), ref, clfid);
		compositionEditor(ref.get(), clfid, name);
	}
}

void EditorUI::ImGuiMemberListener::reflect(Vector3& ref, const char* name) {
	if (icon("V3", "3D Floating-point vector"))
		ImGui::InputFloat3(getShortName(name).c_str(), &ref.x, "%.2f");
}

void EditorUI::ImGuiMemberListener::reflect(Matrix& ref, const char* name) {
	if (icon("Mx", "4x4 transformation matrix")) {
		std::string fullName = getShortName(name);
		for (int i = 0; i < 4; ++i) {
			if (i != 0)
				icon("..", "Matrix continuation");
			ImGui::InputFloat3((fullName + ".Row" + (char)('0' + i)).c_str(), &ref.v[4 * i], "%.2f");
		}
	}
}

void EditorUI::ImGuiMemberListener::reflect(EventNode& ref, const char* name, CKObject* user) {
	if (icon("Ev", "Event sequence node")) {
		auto fullName = getShortName(name);
		EditorUI::IGEventSelector(ui, fullName.c_str(), ref);
	}
}

void EditorUI::ImGuiMemberListener::reflect(MarkerIndex& ref, const char* name) {
	if (icon("Mk", "Marker")) {
		EditorUI::IGMarkerSelector(ui, getShortName(name).c_str(), ref);
	}
}

void EditorUI::ImGuiMemberListener::reflectPostRefTuple(uint32_t& tuple, const char* name) {
	if (icon("PR", "Undecoded object reference (Postponed reference)")) {
		int igtup[3] = { tuple & 63, (tuple >> 6) & 2047, tuple >> 17 };
		if (ImGui::InputInt3(getShortName(name).c_str(), igtup)) {
			tuple = (igtup[0] & 63) | ((igtup[1] & 2047) << 6) | ((igtup[2] & 32767) << 17);
		}
	}
}

void EditorUI::ImGuiMemberListener::reflect(std::string& ref, const char* name) {
	if (icon("St", "Character string")) {
		ImGui::InputText(getShortName(name).c_str(), (char*)ref.c_str(), ref.capacity() + 1, ImGuiInputTextFlags_CallbackResize, GuiUtils::IGStdStringInputCallback, &ref);
	}
}

void EditorUI::ImGuiMemberListener::setNextFlags(MemberFlags flags) {
	currentFlags = flags;
}

void EditorUI::ImGuiMemberListener::enterArray(const char* name) {
	if (icon("[]", "Array") && ImGui::TreeNodeEx(getShortName(name).c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		scopeExpanded.push(true);
	else
		scopeExpanded.push(false);
	NamedMemberListener::enterArray(name);
}

void EditorUI::ImGuiMemberListener::leaveArray() {
	NamedMemberListener::leaveArray();
	if (scopeExpanded.top())
		ImGui::TreePop();
	scopeExpanded.pop();
}

void EditorUI::ImGuiMemberListener::enterStruct(const char* name) {
	if (icon("{}", "Structure") && ImGui::TreeNodeEx(getShortName(name).c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		scopeExpanded.push(true);
	else
		scopeExpanded.push(false);
	NamedMemberListener::enterStruct(name);
}

void EditorUI::ImGuiMemberListener::leaveStruct() {
	NamedMemberListener::leaveStruct();
	if (scopeExpanded.top())
		ImGui::TreePop();
	scopeExpanded.pop();
}

void EditorUI::ImGuiMemberListener::compositionEditor(CKObject* obj, int clfid, const char* name) {
	if (!obj) return;
	if (obj->getClassFullID() == CAnimationDictionary::FULL_ID) {
		AnimDictEditor(ui, (CAnimationDictionary*)obj);
	}
	else if (obj->getClassFullID() == CKSoundDictionaryID::FULL_ID) {
		SoundDictIDEditor(ui, (CKSoundDictionaryID*)obj);
	}
}
