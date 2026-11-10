#include "pch.h"
#include "SavePointFlag.h"
#include "Managers.h"

REGISTER_COMPONENT(SavePointFlag)

SavePointFlag::~SavePointFlag()
{

}

void SavePointFlag::Init()
{
	_savePanel = UI->CreateUI<UIPanel>();
	_savePanel->GetTransform()->SetStretchSize(true);
	_savePanel->GetTransform()->SetSize({ 0.8f, 0.8f });
	_savePanel->SetRenderActive(false);

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
	Managers::Ui.AddUI(_savePanel);
	// _savePanel->SetRenderActive(true);
}
