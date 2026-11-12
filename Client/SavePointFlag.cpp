#include "pch.h"
#include "SavePointFlag.h"
#include "Managers.h"

REGISTER_COMPONENT(SavePointFlag)

SavePointFlag::~SavePointFlag()
{

}

void SavePointFlag::Init()
{
	_checkPointUI = UI->CreateUI<UIFrame>();
	_checkPointUI->GetTransform()->SetStretchSize(true);
	_checkPointUI->GetTransform()->SetSize({ 0.8f, 0.8f });

	_backgroundPanel = UI->CreateUI<UIPanel>();
	_backgroundPanel->SetColor({ 0.2f, 0.2f, 0.2f, 0.5f });
	_backgroundPanel->GetTransform()->SetParent(_checkPointUI->GetTransform());
	_backgroundPanel->GetTransform()->SetStretchSize(true);
	_backgroundPanel->GetTransform()->SetSize({ 1.0f, 1.0f });

	_titleText = UI->CreateUI<UIText>();
	_titleText->GetTransform()->SetParent(_backgroundPanel->GetTransform());
	_titleText->GetTransform()->SetPivot({ 0.0f, 1.0f });
	_titleText->GetTransform()->SetAnchor(UIAnchorMode::LeftTop);
	_titleText->GetTransform()->SetLocalPosition({ 20.0f, -20.0f, 0.0f });
	_titleText->SetSize({ 800.0f, 30.0f });
	_titleText->SetFont(L"KoPubBatang");
	_titleText->SetFontSize(30);
	_titleText->SetText(L"레벨업");
	_titleText->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

	// _levelText = UI->CreateUI<UIText>();

	_checkPointUI->SetRenderActive(false);

	GetGameObject()->SetTag("Interactable");
}

void SavePointFlag::Update()
{

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
	Managers::Ui.AddUI(_checkPointUI);
}
