#include "pch.h"
#include "MiddleGateLeverScript.h"

REGISTER_COMPONENT(MiddleGateLeverScript)

MiddleGateLeverScript::~MiddleGateLeverScript()
{

}

void MiddleGateLeverScript::Init()
{
	_gateObj = RENDER->GetObjectW("MiddleGate");

	GetGameObject()->SetTag("Interactable");
}

void MiddleGateLeverScript::Update()
{
	if (_gateMoveTime > 0.0f) {
		_gateMoveTime -= TIME->DeltaTime();
		_gateObj->GetTransform()->Translate({ 0, 0, TIME->DeltaTime() });
	}
}

void MiddleGateLeverScript::OnDestroy()
{
	_gateObj.reset();
}

void MiddleGateLeverScript::LoadXML(Bulb::XMLElement compElem)
{

}

void MiddleGateLeverScript::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "MiddleGateLeverScript");
}

void MiddleGateLeverScript::Interact(shared_ptr<GameObject> opponent)
{
	if (_isGateOpened) return;

	_gateMoveTime = 2.0f;
	_isGateOpened = true;
}
