#pragma once
#include "Component.h"
class BULB_API Script : public Component
{
	using Super = Component;
public:
	Script();
	virtual ~Script();

	void OnDestroy() override = 0;

	void LoadXML(Bulb::XMLElement compElem) override = 0;
	void SaveXML(Bulb::XMLElement compElem) override = 0;

	shared_ptr<Component> Duplicate() override { return nullptr; }

	ComponentSnapshot CaptureSnapshot() override { 
		ComponentSnapshot snapshot;

		snapshot.id = _id;
		snapshot.componentType = "Script (No Snapshot Setting)";

		return snapshot;
	};
	void RestoreSnapshot(ComponentSnapshot snapshot) override {};

#ifdef BULB_EDITOR
	bool ShowComponentEditorGUI() override;
#endif
};

