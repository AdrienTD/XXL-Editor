#pragma once

#include <cstdint>

struct ImVec4;

namespace EditorUI
{
	struct EditorInterface;

	ImVec4 getPFCellColor(uint8_t val);
	void IGPathfindingEditor(EditorInterface& ui);
}