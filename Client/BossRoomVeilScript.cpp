 #include "pch.h"
#include "BossRoomVeilScript.h"
#include "PlayerScript.h"

REGISTER_COMPONENT(BossRoomVeilScript)

void BossRoomVeilScript::Init()
{
	GetGameObject()->SetTag("Interactable");

	_veilEnterSnd = SOUND->LoadSound("Sounds/VeilEnter.wav", false);

	_veilAs = GetGameObject()->GetComponent<AudioSource>();
	_veilAs->SetSound(_veilEnterSnd);
	_veilAs->SetAsBGM(true);
}

void BossRoomVeilScript::Interact(shared_ptr<GameObject> opponent)
{
	isInteractable = false;

	opponent->GetComponent<PlayerScript>()->SetState(PlayerState::ENTER_VEIL);
	_veilAs->Play();
}

void BossRoomVeilScript::OnDestroy()
{
	_veilAs.reset();

	_veilEnterSnd->release();
}

void BossRoomVeilScript::LoadXML(Bulb::XMLElement compElem)
{

}

void BossRoomVeilScript::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "BossRoomVeilScript");
}
