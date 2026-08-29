#include "pch.h"
#include "Script.h"

Script::Script() : Super(ComponentType::Script)
{

}

Script::~Script()
{
	cout << "Released - Script:" << _id << "\n";
}

#ifdef BULB_EDITOR
bool Script::ShowComponentEditorGUI()
{
	if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen)) {
		string name = typeid(*this).name();
		name.erase(0, 6);
		if (ImGui::BeginCombo("Script ##ScriptFileCombo", name.c_str())) {

			ImGui::EndCombo();
		}
	}

	return false;
}
#endif
