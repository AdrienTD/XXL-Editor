#pragma once

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <nlohmann/json.hpp>

struct Window;

struct Encyclopedia {
	Encyclopedia();
	~Encyclopedia();

	void setKVersion(int ver);
	void load();
	void clear();
	const nlohmann::json* getClassJson(int clsFullID);
	const nlohmann::json* getEventJson(int fid, int event);
	const nlohmann::json* getEventJson(std::span<const int> fids, int event);
	std::string getEventName(const nlohmann::json* ev, int event);
	static std::pair<int, int> decodeRange(std::string_view sv);
	const nlohmann::json* getBeaconJson(int beaconTypeId);

	std::unordered_map<int, nlohmann::json> kclasses;
	std::unordered_map<std::string, nlohmann::json> eventSets;
	std::unordered_map<int, nlohmann::json> beaconInfos;
	int kversion = 1;
	bool loaded = false;

	Window* window = nullptr;
private:
	void loadFile(const std::string& filename);
};