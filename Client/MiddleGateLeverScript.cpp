#include "pch.h"
#include "MiddleGateLeverScript.h"
#include "PlayerScript.h"

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

	isInteractable = false;

	_gateMoveTime = 2.0f;
	_isGateOpened = true;

	shared_ptr<Transform> playerTransform = opponent->GetTransform();
	Bulb::Vector3 pos = GetTransform()->GetPosition();
	Bulb::Vector3 playerPos = playerTransform->GetPosition();
	pos.y = playerPos.y;
	playerPos = pos - Bulb::Vector3{ 1.0f, 0.0f, 0.0f };

	playerTransform->SetPosition(playerPos);
	playerTransform->LookAtWithNoRoll(playerPos - (pos - playerPos));
	opponent->GetComponent<PlayerScript>()->SetState(PlayerMovementState::INTERACT);
}
