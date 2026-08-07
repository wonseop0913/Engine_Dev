#include "pch.h"
#include "BossRoomGateLeverScript.h"
#include "PlayerScript.h"

REGISTER_COMPONENT(BossRoomGateLeverScript)

BossRoomGateLeverScript::~BossRoomGateLeverScript()
{

}

void BossRoomGateLeverScript::Init()
{
	_gateObj = RENDER->GetObjectW("Boss Room Block Gate");

	GetGameObject()->SetTag("Interactable");
}

void BossRoomGateLeverScript::Update()
{

}

void BossRoomGateLeverScript::OnDestroy()
{
	_gateObj.reset();
}

void BossRoomGateLeverScript::LoadXML(Bulb::XMLElement compElem)
{

}

void BossRoomGateLeverScript::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "BossRoomGateLeverScript");
}

void BossRoomGateLeverScript::Interact(shared_ptr<GameObject> opponent)
{
	if (_isGateOpened) return;

	isInteractable = false;

	_gateObj->SetActive(false);

	_isGateOpened = true;

	shared_ptr<Transform> playerTransform = opponent->GetTransform();
	Bulb::Vector3 pos = GetTransform()->GetPosition();
	Bulb::Vector3 playerPos = playerTransform->GetPosition();
	pos.y = playerPos.y;
	playerPos = pos - Bulb::Vector3{ 0.0f, 0.0f, 1.0f };

	playerTransform->SetPosition(playerPos);
	playerTransform->LookAtWithNoRoll(playerPos - (pos - playerPos));
	opponent->GetComponent<PlayerScript>()->SetState(PlayerMovementState::INTERACT);
}
