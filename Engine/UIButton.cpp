#include "pch.h"
#include "UIButton.h"

UIButton::UIButton() : UIElement(UIType::Button)
{
	_name = "UIButton";

	background = UI->CreateUI<UIPanel>();
	text = UI->CreateUI<UIText>();
}

UIButton::~UIButton()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - UIButton\n";
#endif

	background.reset();
	text.reset();
}

void UIButton::Init()
{
	_transform->SetSize({ 200.0f, 60.0f });

	background->SetColor(colorDefault);
	background->GetTransform()->SetParent(_transform);
	background->GetTransform()->SetStretchSize(true);
	background->GetTransform()->SetSizePercentage({ 1.0f, 1.0f });
	background->GetTransform()->SetDepth(_transform->GetDepth() + 1.0f);

	text->GetTransform()->SetParent(_transform);
	text->GetTransform()->SetStretchSize(true);
	text->GetTransform()->SetSizePercentage({ 1.0f, 1.0f });
}

void UIButton::Update()
{

}

void UIButton::OnDestroy()
{
	mouseEnterEvent.Clear();
	mouseExitEvent.Clear();
	mouseDownEvent.Clear();
}

void UIButton::SetButtonActive(bool active)
{
	if (_buttonActive == active) return;

	_buttonActive = active;
	if (!_buttonActive) {
		UIElement::OnMouseExit();
		background->SetColor(colorDisabled);
	}
	else {
		background->SetColor(colorDefault);
	}
}

void UIButton::LoadXML(XMLElement* uiElem)
{
	XMLElement* textElem = uiElem->FirstChildElement("Text");
	string textStr(textElem->Attribute("Text"));
	text->SetText(textStr);
}

void UIButton::OnMouseEnter()
{
	UIElement::OnMouseEnter();
	if (!_buttonActive) return;
	background->SetColor(colorHovered);
	mouseEnterEvent.Execute();
}

void UIButton::OnMouseExit()
{
	UIElement::OnMouseExit();
	if (!_buttonActive) return;
	background->SetColor(colorDefault);
	mouseExitEvent.Execute();
}

void UIButton::OnMouseDown()
{
	if (!_buttonActive) return;
	UIElement::OnMouseDown();
	background->SetColor(colorClicked);
	mouseDownEvent.Execute();
}
