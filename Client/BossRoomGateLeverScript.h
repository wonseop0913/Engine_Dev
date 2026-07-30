#pragma once
#include "Script.h"

class BossRoomGateLeverScript : public Script
{
public:
	~BossRoomGateLeverScript();

	void Init() override;

	void Update() override;

	void OnCollision(shared_ptr<GameObject> other) override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;

	void SaveXML(Bulb::XMLElement compElem) override;

private:
	shared_ptr<GameObject> _gateObj;
	float _gateMoveTime = 0.0f;

	bool _isGateOpened = false;
};

