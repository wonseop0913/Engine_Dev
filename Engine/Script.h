#pragma once
#include "Component.h"
class BULB_API Script : public Component
{
	using Super = Component;
public:
	Script();
	virtual ~Script();

#ifdef BULB_EDITOR
	virtual bool ShowComponentEditorGUI() override { return false; }
#endif

	virtual void OnDestroy() override = 0;

	virtual void LoadXML(Bulb::XMLElement compElem) override = 0;
	virtual void SaveXML(Bulb::XMLElement compElem) override = 0;

	shared_ptr<Component> Duplicate() override { return nullptr; }

	virtual ComponentSnapshot CaptureSnapshot() override { 
		ComponentSnapshot snapshot;

		snapshot.id = _id;
		snapshot.componentType = "Script (No Snapshot Setting)";

		return snapshot;
	};
	virtual void RestoreSnapshot(ComponentSnapshot snapshot) override { };
};

