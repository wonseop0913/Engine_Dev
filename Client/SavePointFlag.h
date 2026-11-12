#pragma once
#include "Interactable.h"

class SavePointFlag : public Interactable {
public:
	~SavePointFlag();

	void Init() override;

	void Update() override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;

	void SaveXML(Bulb::XMLElement compElem) override;

	void Interact(shared_ptr<GameObject> opponent) override;

private:
	shared_ptr<UIFrame> _checkPointUI;
	shared_ptr<UIPanel> _backgroundPanel;
	shared_ptr<UIText> _titleText;
	shared_ptr<UIText> _levelText;
};
