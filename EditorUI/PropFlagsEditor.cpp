#include "PropFlagsEditor.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <imgui/imgui.h>

bool PropFlagsEditor(unsigned int& flagsValue, const nlohmann::json& flagsInfo) {
	bool modified = false;

	std::vector<std::tuple<int, int, const nlohmann::json*>> flagsList;
	flagsList.reserve(flagsInfo.size());
	for (const auto& [key, jsobj] : flagsInfo.items()) {
		auto sep = key.find('-');
		int bitStartIndex = 0, bitEndIndex = 0;
		if (sep == key.npos) {
			std::from_chars(key.data(), key.data() + key.size(), bitStartIndex);
			bitEndIndex = bitStartIndex;
		}
		else {
			std::from_chars(key.data(), key.data() + sep, bitStartIndex);
			std::from_chars(key.data() + sep + 1, key.data() + key.size(), bitEndIndex);
		}
		flagsList.emplace_back(bitStartIndex, bitEndIndex, &jsobj);
	}
	std::ranges::sort(flagsList, {}, [](const auto& key) { return std::get<0>(key); });

	for (const auto& [bitStartIndex, bitEndIndex, jsptr] : flagsList) {
		const auto& jsobj = *jsptr;
		unsigned int mask = ((1 << (bitEndIndex - bitStartIndex + 1)) - 1) << bitStartIndex;
		if (jsobj.is_string()) {
			const auto& name = jsobj.get_ref<const std::string&>();
			if (bitStartIndex == bitEndIndex) {
				modified |= ImGui::CheckboxFlags(name.c_str(), &flagsValue, 1 << bitStartIndex);
			}
			else {
				unsigned int v = (flagsValue & mask) >> bitStartIndex;
				ImGui::SetNextItemWidth(48.0f);
				bool b = ImGui::InputScalar(name.c_str(), ImGuiDataType_U32, &v);
				if (b) {
					modified = true;
					flagsValue = (flagsValue & ~mask) | ((v << bitStartIndex) & mask);
				}
			}
		}
		else if (jsobj.is_object()) {
			unsigned int v = (flagsValue & mask) >> bitStartIndex;
			const auto& name = jsobj.at("name").get_ref<const std::string&>();
			std::string preview = std::to_string(v);
			if (auto it = jsobj.find(preview); it != jsobj.end())
				preview = it->get_ref<const std::string&>();
			bool b = false;
			if (ImGui::BeginCombo(name.c_str(), preview.c_str())) {
				b |= ImGui::InputScalar("Value", ImGuiDataType_U32, &v);
				for (auto& [ek, ev] : jsobj.items()) {
					if (ek != "name") {
						if (ImGui::Selectable(ev.get_ref<const std::string&>().c_str())) {
							v = std::stoi(ek);
							b = true;
						}
					}
				}
				ImGui::EndCombo();
			}
			if (b) {
				modified = true;
				flagsValue = (flagsValue & ~mask) | ((v << bitStartIndex) & mask);
			}

		}
	}
	return modified;
}
