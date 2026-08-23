#pragma once
#include "Interactable.h"

class BossRoomGateLeverScript : public Interactable
{
public:
	~BossRoomGateLeverScript();

	void Init() override;

	void Update() override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;

	void SaveXML(Bulb::XMLElement compElem) override;

	void Interact(shared_ptr<GameObject> opponent) override;

private:
	shared_ptr<GameObject> _gateObj;
	shared_ptr<Transform> _stickTransform;
	shared_ptr<AudioSource> _leverAs;

	float _gateMoveTime = 0.0f;
	bool _stickRotate = false;

	bool _isGateOpened = false;
};

