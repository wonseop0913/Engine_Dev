#include "pch.h"
#include "BossRoomGateLeverScript.h"
#include "PlayerScript.h"
#include "MainSceneScript.h"

REGISTER_COMPONENT(BossRoomGateLeverScript)

BossRoomGateLeverScript::~BossRoomGateLeverScript()
{

}

void BossRoomGateLeverScript::Init()
{
	_gateObj = RENDER->GetObjectW("Boss Room Block Gate");
	_stickTransform = GetTransform()->GetChild("Stick");
	_leverAs = GetGameObject()->GetComponent<AudioSource>();

	GetGameObject()->SetTag("Interactable");
}

void BossRoomGateLeverScript::Update()
{
	if (_stickRotate) {
		if (_stickTransform->GetLocalRotation().x <= -45.0f) {
			_stickTransform->SetLocalRotation({ -45.0f, 0.0f, 0.0f });
			_stickRotate = false;
		}
		else {
			_stickTransform->Rotate(Bulb::Vector3(-TIME->DeltaTime() * 3.0f, 0.0f, 0.0f));
		}
	}
}

void BossRoomGateLeverScript::OnDestroy()
{
	_gateObj.reset();
	_stickTransform.reset();
	_leverAs.reset();
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
	_stickRotate = true;

	_isGateOpened = true;

	shared_ptr<Transform> playerTransform = opponent->GetTransform();
	Bulb::Vector3 pos = GetTransform()->GetPosition();
	Bulb::Vector3 playerPos = playerTransform->GetPosition();
	pos.y = playerPos.y;
	playerPos = pos - Bulb::Vector3{ 0.0f, 0.0f, 1.0f };

	playerTransform->SetPosition(playerPos);
	playerTransform->LookAtWithNoRoll(playerPos - (pos - playerPos));
	opponent->GetComponent<PlayerScript>()->SetState(PlayerState::INTERACT);

	_leverAs->Play();

	RENDER->GetObjectW("SceneScript")->GetComponent<MainSceneScript>()->SetInfoPanel(L"어딘가의 통로가 개방됐다.");
}
