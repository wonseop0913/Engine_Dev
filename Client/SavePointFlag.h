#pragma once
#include "Interactable.h"

class SavePointFlag : public Interactable {
public:
	~SavePointFlag() = default;

	void Init() override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;

	void SaveXML(Bulb::XMLElement compElem) override;

	void Interact(shared_ptr<GameObject> opponent) override;

public:
	void OnClickedLevelUpButton();

private:
	void UpdateLevelUpUI();

	shared_ptr<UIFrame> _checkPointUI;
	shared_ptr<UIPanel> _backgroundPanel;
	shared_ptr<UIText> _titleText;
	shared_ptr<UIText> _levelText;
	shared_ptr<UIText> _currencyText;
	shared_ptr<UIText> _levelUpCostText;

	shared_ptr<UIButton> _levelUpBtn;
};
