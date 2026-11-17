#pragma once
#include "UIElement.h"

class BULB_API UIButton : public UIElement
{
	friend class UIManager;
public:
	UIButton();
	~UIButton();

	void Init() override;
	void Update() override;

	void OnDestroy() override;

	void SetButtonActive(bool active);
	bool IsButtonActive() const { return _buttonActive; }

	void LoadXML(XMLElement* uiElem) override;

	void OnMouseEnter() override;
	void OnMouseExit() override;
	void OnMouseDown() override;

public:
	Event<> mouseEnterEvent;
	Event<> mouseExitEvent;
	Event<> mouseDownEvent;

	shared_ptr<UIPanel> background;
	shared_ptr<UIText> text;

	Bulb::Color colorDefault = { 1.0f, 1.0f, 1.0f, 1.0f };
	Bulb::Color colorHovered = { 0.7f, 0.7f, 0.7f, 1.0f };
	Bulb::Color colorClicked = { 0.55f, 0.55f, 0.55f, 1.0f };
	Bulb::Color colorDisabled = { 0.4f, 0.4f, 0.4f, 1.0f };

private:
	bool _buttonActive = true;
};

