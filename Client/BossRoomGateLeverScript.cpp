#include "pch.h"
#include "BossRoomGateLeverScript.h"

REGISTER_COMPONENT(BossRoomGateLeverScript)

BossRoomGateLeverScript::~BossRoomGateLeverScript()
{

}

void BossRoomGateLeverScript::Init()
{
	_gateObj = RENDER->GetObjectW("Boss Room Block Gate Plain");

	GetGameObject()->SetTag("Interactable");
}

void BossRoomGateLeverScript::Update()
{

}

void BossRoomGateLeverScript::OnCollision(shared_ptr<GameObject> other)
{
	//if (_isGateOpened) return;

	//if (other->GetTag() == "Player") {
	//	if (INPUTM->IsKeyDown(KeyValue::E)) {
	//		_gateObj->SetActive(false);

	//		_isGateOpened = true;
	//	}
	//}
}

void BossRoomGateLeverScript::OnDestroy()
{

}

void BossRoomGateLeverScript::LoadXML(Bulb::XMLElement compElem)
{

}

void BossRoomGateLeverScript::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "BossRoomGateLeverScript");
}

void BossRoomGateLeverScript::Interact()
{
	if (_isGateOpened) return;

	_gateObj->SetActive(false);

	_isGateOpened = true;
}
