#pragma once
#include "Script.h"

class ZombieScript : public Script
{
public:
	void Init() override;

	void Update() override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;

	void SaveXML(Bulb::XMLElement compElem) override;
};

