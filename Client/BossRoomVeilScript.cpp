 #include "pch.h"
#include "BossRoomVeilScript.h"
#include "PlayerScript.h"

REGISTER_COMPONENT(BossRoomVeilScript)

void BossRoomVeilScript::Init()
{
	GetGameObject()->SetTag("Interactable");
}

void BossRoomVeilScript::Interact(shared_ptr<GameObject> opponent)
{
	isInteractable = false;

	opponent->GetComponent<PlayerScript>()->SetState(PlayerMovementState::ENTER_VEIL);
}

void BossRoomVeilScript::OnDestroy()
{

}

void BossRoomVeilScript::LoadXML(Bulb::XMLElement compElem)
{

}

void BossRoomVeilScript::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "BossRoomVeilScript");
}
