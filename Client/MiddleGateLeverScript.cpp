#include "pch.h"
#include "MiddleGateLeverScript.h"

REGISTER_COMPONENT(MiddleGateLeverScript)

MiddleGateLeverScript::~MiddleGateLeverScript()
{

}

void MiddleGateLeverScript::Init()
{
	_middleGateObj = RENDER->GetObjectW("MiddleGate");
}

void MiddleGateLeverScript::Update()
{
	if (_middleGateMoveTime > 0.0f) {
		_middleGateMoveTime -= TIME->DeltaTime();
		_middleGateObj->GetTransform()->Translate({ 0, 0, TIME->DeltaTime() });
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

void MiddleGateLeverScript::OnCollisionEnter(shared_ptr<GameObject> other)
{

}

void MiddleGateLeverScript::OnCollision(shared_ptr<GameObject> other)
{
	if (_isMiddleGateOpened) return;

	if (other->GetTag() == "Player") {
		if (INPUTM->IsKeyDown(KeyValue::E)) {
			_middleGateMoveTime = 2.0f;
			_isMiddleGateOpened = true;
		}
	}
}
