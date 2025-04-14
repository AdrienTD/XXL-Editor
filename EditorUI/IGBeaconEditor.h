#pragma once

#include "KEnvironment.h"

namespace EditorUI
{
	struct EditorInterface;

	const int maxGameSupportingAdvancedBeaconEditing = KEnvironment::KVERSION_XXL1;

	void IGBeaconEditor(EditorInterface& ui);
}