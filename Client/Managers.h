#pragma once

#include "UIManager.h"
#include "GameManager.h"

class Managers : public Script
{
public:
	static Client::UIManager Ui;
	static Client::GameManager Game;
private:
	static Managers* Manager;

public:
	~Managers();

	void Init() override;

	void Update() override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;

	void SaveXML(Bulb::XMLElement compElem) override;
};
