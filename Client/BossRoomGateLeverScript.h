#pragma once
#include "Interactable.h"

class BossRoomGateLeverScript : public Interactable
{
public:
	~BossRoomGateLeverScript();

	void Init() override;

	void Update() override;

	void OnCollision(shared_ptr<GameObject> other) override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;

	void SaveXML(Bulb::XMLElement compElem) override;

	void Interact() override;

private:
	shared_ptr<GameObject> _gateObj;
	float _gateMoveTime = 0.0f;

	bool _isGateOpened = false;
};

