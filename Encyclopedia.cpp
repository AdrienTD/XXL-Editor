#include "Encyclopedia.h"
#include <charconv>
#include "File.h"

#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include "GuiUtils.h"

Encyclopedia::Encyclopedia() = default;
Encyclopedia::~Encyclopedia() = default;

void Encyclopedia::setKVersion(int ver)
{
	if (kversion != ver) {
		clear();
		kversion = ver;
	}
}

void Encyclopedia::load()
{
	if (loaded) return;
	clear();

	try {
		auto [enPtr, enSize] = GetResourceContent("Encyclopedia.json");
		if (enPtr && enSize > 0) {
			nlohmann::json jsRoot = nlohmann::json::parse(static_cast<const char*>(enPtr), static_cast<const char*>(enPtr) + enSize);
			auto& jsGame = jsRoot.at("games").at(std::to_string(kversion));
			auto& jsFileList = jsGame.at("classInfoFiles");
			for (auto& file : jsFileList)
				loadFile(file.get_ref<const std::string&>());
		}
	}
	catch (const std::exception& ex) {
		if (window)
			GuiUtils::MsgBox(window, ex.what(), 16);
	}
	loaded = true;
}

void Encyclopedia::clear()
{
	kclasses.clear();
	eventSets.clear();
	loaded = false;
}

const nlohmann::json* Encyclopedia::getClassJson(int clsFullID)
{
	load();
	auto it = kclasses.find(clsFullID);
	if (it != kclasses.end())
		return &it->second;
	else
		return nullptr;
}

const nlohmann::json* Encyclopedia::getEventJson(int fid, int event)
{
	load();
	if (auto it = kclasses.find(fid); it != kclasses.end()) {
		if (auto isit = it->second.find("events"); isit != it->second.end()) {
			for (auto& ev : isit.value()) {
				auto [idStart, idEnd] = decodeRange(ev.at("id").get_ref<const std::string&>());
				if (idStart <= event && event <= idEnd) {
					return &ev;
				}
			}
		}
		if (auto isit = it->second.find("includeSets"); isit != it->second.end()) {
			for (auto& setname : isit.value()) {
				if (auto esit = eventSets.find(setname.get_ref<const std::string&>()); esit != eventSets.end()) {
					for (auto& ev : esit->second.at("events")) {
						auto [idStart, idEnd] = decodeRange(ev.at("id").get_ref<const std::string&>());
						if (idStart <= event && event <= idEnd) {
							return &ev;
						}
					}
				}
			}
		}
	}
	return nullptr;
}

std::string Encyclopedia::getEventName(const nlohmann::json* ev, int event)
{
	if (ev)
		return fmt::format("{:04X}: {}", event, ev->at("name").get_ref<const std::string&>());
	return fmt::format("{:04X}: Unknown action", event);
}

void Encyclopedia::loadFile(const std::string& filename)
{
	auto [enPtr, enSize] = GetResourceContent(filename.c_str());
	assert(enPtr && enSize > 0);
	nlohmann::json jsRoot = nlohmann::json::parse(static_cast<const char*>(enPtr), static_cast<const char*>(enPtr) + enSize);
	
	// Classes
	if (auto itClassList = jsRoot.find("classes"); itClassList != jsRoot.end()) {
		for (auto& jsClass : *itClassList) {
			int clcat = jsClass.at("category").get<int>();
			int clid = jsClass.at("id").get<int>();
			int fullid = clcat | (clid << 6);
			auto& enClass = kclasses[fullid];
			for (auto& [key, val] : jsClass.items()) {
				enClass[key] = std::move(val);
			}
		}
	}

	// Event sets
	if (auto itEventSets = jsRoot.find("eventSets"); itEventSets != jsRoot.end()) {
		for (auto& elem : *itEventSets) {
			eventSets.insert_or_assign(elem.at("name").get<std::string>(), std::move(elem));
		}
	}

}

std::pair<int, int> Encyclopedia::decodeRange(std::string_view sv) {
	int a;
	assert(sv.size() == 4 || sv.size() == 9);
	std::from_chars(sv.data(), sv.data() + 4, a, 16);
	if (sv.size() == 9 && sv[4] == '-') {
		int b;
		std::from_chars(sv.data() + 5, sv.data() + 9, b, 16);
		return { a, b };
	}
	return { a, a };
}
