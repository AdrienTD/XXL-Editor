#pragma once

namespace EditorUI
{
	struct EditorInterface;

	void IGSceneNodeEditor(EditorInterface& ui);

	void IGSceneGraph(EditorInterface& ui);
	void IGSceneNodeProperties(EditorInterface& ui);
}