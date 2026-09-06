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
	_stickTransform = GetTransform()->GetChild("Stick");
	_gateAs = _gateObj->GetComponent<AudioSource>();

	GetGameObject()->SetTag("Interactable");

	_leverAs = GetGameObject()->GetComponent<AudioSource>();
}

void MiddleGateLeverScript::Update()
{
	if (_gateMoveTime > 0.0f) {
		_gateMoveTime -= TIME->DeltaTime();
		_gateObj->GetTransform()->Translate({ 0, 0, TIME->DeltaTime() / 5.0f });
	}

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

void MiddleGateLeverScript::OnDestroy()
{
	_gateObj.reset();
	_stickTransform.reset();
	_leverAs.reset();
	_gateAs.reset();
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

	_gateMoveTime = 10.0f;
	_stickRotate = true;
	_isGateOpened = true;

	shared_ptr<Transform> playerTransform = opponent->GetTransform();
	Bulb::Vector3 pos = GetTransform()->GetPosition();
	Bulb::Vector3 playerPos = playerTransform->GetPosition();
	pos.y = playerPos.y;
	playerPos = pos - Bulb::Vector3{ 1.0f, 0.0f, 0.0f };

	playerTransform->SetPosition(playerPos);
	playerTransform->LookAtWithNoRoll(playerPos - (pos - playerPos));
	opponent->GetComponent<PlayerScript>()->SetState(PlayerState::INTERACT);

	_leverAs->Play();
	_gateAs->Play();
}
