#pragma once
#include "Script.h"

class MiddleGateLeverScript : public Script
{
public:
	~MiddleGateLeverScript();

	void Init() override;
	void Update() override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;
	void SaveXML(Bulb::XMLElement compElem) override;

	void OnCollisionEnter(shared_ptr<GameObject> other) override;

	void OnCollision(shared_ptr<GameObject> other) override;

private:
	shared_ptr<GameObject> _middleGateObj;
	float _middleGateMoveTime = 0.0f;

	bool _isMiddleGateOpened = false;
};

