#pragma once

#include "KEnvironment.h"
#include "CKUtils.h"

namespace EditorUI
{
	struct EditorInterface;

	// Creates ImGui editing widgets for every member in a member-reflected object
	struct ImGuiMemberListener : NamedMemberListener {
		KEnvironment& kenv; EditorInterface& ui;
		MemberFlags currentFlags = MemberFlags::MF_NONE;
		std::stack<bool> scopeExpanded;

		ImGuiMemberListener(KEnvironment& kenv, EditorInterface& ui) : kenv(kenv), ui(ui) {}

		bool icon(const char* label, const char* desc = nullptr);

		template<std::integral T> void flagsEditor(const char* name, T& value);

		void reflect(uint8_t& ref, const char* name) override;
		void reflect(uint16_t& ref, const char* name) override;
		void reflect(uint32_t& ref, const char* name) override;
		void reflect(int8_t& ref, const char* name) override;
		void reflect(int16_t& ref, const char* name) override;
		void reflect(int32_t& ref, const char* name) override;
		void reflect(float& ref, const char* name) override;
		void reflectAnyRef(kanyobjref& ref, int clfid, const char* name) override;
		void reflect(Vector3& ref, const char* name) override;
		void reflect(Matrix& ref, const char* name) override;
		void reflect(EventNode& ref, const char* name, CKObject* user) override;
		void reflect(MarkerIndex& ref, const char* name) override;
		void reflectPostRefTuple(uint32_t& tuple, const char* name) override;
		void reflect(std::string& ref, const char* name) override;

		void setNextFlags(MemberFlags flags) override;

		void enterArray(const char* name) override;
		void leaveArray() override;

		void enterStruct(const char* name) override;
		void leaveStruct() override;

		void compositionEditor(CKObject* obj, int clfid, const char* name);
	};
}