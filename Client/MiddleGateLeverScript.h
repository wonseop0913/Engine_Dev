#pragma once
#include "Interactable.h"

class MiddleGateLeverScript : public Interactable
{
public:
	~MiddleGateLeverScript();

	void Init() override;
	void Update() override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;
	void SaveXML(Bulb::XMLElement compElem) override;

	void Interact(shared_ptr<GameObject> opponent) override;

private:
	shared_ptr<GameObject> _gateObj;
	float _gateMoveTime = 0.0f;

	bool _isGateOpened = false;
};

