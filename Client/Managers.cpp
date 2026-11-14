#include "pch.h"
#include "Managers.h"

REGISTER_COMPONENT(Managers)

Managers* Managers::Manager;

Client::UIManager Managers::Ui;
Client::GameManager Managers::Game;


Managers::~Managers()
{

}

void Managers::Init()
{
	if (Manager != nullptr) {
		GetGameObject()->Delete();
		return;
	}
	Manager = this;
}

void Managers::Update()
{
	Ui.Update();
}

void Managers::OnDestroy()
{

}

void Managers::LoadXML(Bulb::XMLElement compElem)
{

}

void Managers::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "Managers");
}
