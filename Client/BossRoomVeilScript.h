#pragma once
#include "Interactable.h"
class BossRoomVeilScript : public Interactable
{
public:
	void Init() override;

	void Interact(shared_ptr<GameObject> opponent) override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;

	void SaveXML(Bulb::XMLElement compElem) override;

private:
	AudioClip _veilEnterSnd;
	shared_ptr<AudioSource> _veilAs;
};

