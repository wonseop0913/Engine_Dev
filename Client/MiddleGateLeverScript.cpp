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

}

void MiddleGateLeverScript::LoadXML(Bulb::XMLElement compElem)
{

}

void MiddleGateLeverScript::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "MiddleGateLeverScript");
}

void MiddleGateLeverScript::OnCollision(shared_ptr<GameObject> other)
{
	//if (_isGateOpened) return;

	//if (other->GetTag() == "Player") {
	//	if (INPUTM->IsKeyDown(KeyValue::E)) {
	//		_gateMoveTime = 2.0f;
	//		_isGateOpened = true;
	//	}
	//}
}

void MiddleGateLeverScript::Interact()
{
	if (_isGateOpened) return;

	_gateMoveTime = 2.0f;
	_isGateOpened = true;
}
