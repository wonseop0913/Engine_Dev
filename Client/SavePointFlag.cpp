#include "pch.h"
#include "SavePointFlag.h"
#include "Managers.h"

REGISTER_COMPONENT(SavePointFlag)

void SavePointFlag::Init()
{
	_checkPointUI = UI->CreateUI<UIFrame>();
	_checkPointUI->GetTransform()->SetStretchSize(true);
	_checkPointUI->GetTransform()->SetSizePercentage({ 0.8f, 0.8f });

	_backgroundPanel = UI->CreateUI<UIPanel>();
	_backgroundPanel->SetColor({ 0.2f, 0.2f, 0.2f, 0.5f });
	_backgroundPanel->GetTransform()->SetParent(_checkPointUI->GetTransform());
	_backgroundPanel->GetTransform()->SetStretchSize(true);
	_backgroundPanel->GetTransform()->SetSizePercentage({ 1.0f, 1.0f });
	_backgroundPanel->GetTransform()->SetDepth(5);

	_titleText = UI->CreateUI<UIText>();
	_titleText->GetTransform()->SetParent(_backgroundPanel->GetTransform());
	_titleText->GetTransform()->SetPivot({ 0.0f, 1.0f });
	_titleText->GetTransform()->SetAnchor(UIAnchorMode::LeftTop);
	_titleText->GetTransform()->SetLocalPosition({ 20.0f, -20.0f, 0.0f });
	_titleText->GetTransform()->SetDepth(1);
	_titleText->SetSize({ 800.0f, 30.0f });
	_titleText->SetFont(L"KoPubBatang");
	_titleText->SetFontSize(30);
	_titleText->SetText(L"레벨업");
	_titleText->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

	_levelText = UI->CreateUI<UIText>();
	_levelText->GetTransform()->SetParent(_backgroundPanel->GetTransform());
	_levelText->GetTransform()->SetPivot({ 0.0f, 1.0f });
	_levelText->GetTransform()->SetAnchor(UIAnchorMode::LeftTop);
	_levelText->GetTransform()->SetLocalPosition({ 20.0f, -60.0f, 0.0f });
	_levelText->GetTransform()->SetDepth(1);
	_levelText->SetSize({ 800.0f, 30.0f });
	_levelText->SetFont(L"KoPubBatang");
	_levelText->SetFontSize(24);
	_levelText->SetText(L"");
	_levelText->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

	_levelUpBtn = UI->CreateUI<UIButton>();
	_levelUpBtn->GetTransform()->SetParent(_backgroundPanel->GetTransform());
	_levelUpBtn->GetTransform()->SetPivot({ 0.0f, 1.0f });
	_levelUpBtn->GetTransform()->SetAnchor(UIAnchorMode::LeftTop);
	_levelUpBtn->GetTransform()->SetLocalPosition({ 20.0f, -100.0f, -1.0f });
	_levelUpBtn->GetTransform()->SetDepth(1);
	// _levelUpBtn->GetTransform()->SetDepth(3.0f);
	// _levelUpBtn->background->SetTexture(L"..\\Resources\\Textures\\UI\\ButtonHovered.png");
	_levelUpBtn->GetTransform()->SetSize({ 200.0f, 75.0f });
	_levelUpBtn->text->SetFont("Georgia");
	_levelUpBtn->text->SetText("Up");
	_levelUpBtn->mouseEnterEvent += [this]() { OnMouseEnterButton(); };
	_levelUpBtn->mouseDownEvent += [this]() { OnClickedLevelUpButton(); };

	_checkPointUI->SetRenderActive(false);

	GetGameObject()->SetTag("Interactable");
}

void SavePointFlag::OnDestroy()
{

}

void SavePointFlag::LoadXML(Bulb::XMLElement compElem)
{

}

void SavePointFlag::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "SavePointFlag");
}

void SavePointFlag::Interact(shared_ptr<GameObject> opponent)
{
	_levelText->SetText(to_string(Managers::Game.GetLevel()));
	Managers::Ui.AddUI(_checkPointUI);
}

void SavePointFlag::OnMouseEnterButton()
{
	// __debugbreak();
}

void SavePointFlag::OnClickedLevelUpButton()
{
	if (Managers::Game.LevelUp())
		_levelText->SetText(to_string(Managers::Game.GetLevel()));
}
