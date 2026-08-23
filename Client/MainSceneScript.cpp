#include "pch.h"
#include "MainSceneScript.h"
#include "BossScript.h"

REGISTER_COMPONENT(MainSceneScript)

MainSceneScript::~MainSceneScript()
{

}

void MainSceneScript::Init()
{
	_states.push_back(new FadeIn());
	_states.push_back(new Common());
	_states.push_back(new FadeOut());
	_states.push_back(new BossFight());
	SetState(MainSceneState::FadeIn);

	_mainAs = GetGameObject()->GetComponent<AudioSource>();
}

void MainSceneScript::Update()
{
	if (_isStateChanged) {
		_states[static_cast<int>(_currentState)]->StateStart(this);
		_isStateChanged = false;
	}

	_states[static_cast<int>(_currentState)]->StateUpdate(this);
}

void MainSceneScript::OnDestroy()
{
	for (int i = 0; i < _states.size(); ++i) {
		delete _states[i];
	}
}

void MainSceneScript::LoadXML(Bulb::XMLElement compElem)
{

}

void MainSceneScript::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "MainSceneScript");
}

ComponentSnapshot MainSceneScript::CaptureSnapshot()
{
	ComponentSnapshot snapshot;
	snapshot.id = _id;
	snapshot.componentType = "MainSceneScript";

	return snapshot;
}

void MainSceneScript::RestoreSnapshot(ComponentSnapshot snapshot)
{
	SetState(MainSceneState::FadeIn);
}

void MainSceneScript::FadeIn::StateStart(MainSceneScript* owner)
{
	Camera::GetCurrentCamera()->SetColorBlend({ 0.0f, 0.0f, 0.0f, 1.0f });
	Camera::GetCurrentCamera()->SetColorBlend({ 0.0f, 0.0f, 0.0f, 0.0f }, owner->_fadeInTime);
	_elapsedTime = 0.0f;
}

void MainSceneScript::FadeIn::StateUpdate(MainSceneScript* owner)
{
	_elapsedTime += TIME->DeltaTime();
	if (_elapsedTime >= owner->_fadeInTime) {
		_elapsedTime = 0.0f;
		owner->SetState(MainSceneState::Common);
	}
}

void MainSceneScript::FadeOut::StateStart(MainSceneScript* owner)
{
	Camera::GetCurrentCamera()->SetColorBlend({ 0.0f, 0.0f, 0.0f, 1.0f }, owner->_fadeOutTime);
	_elapsedTime = 0.0f;
}

void MainSceneScript::FadeOut::StateUpdate(MainSceneScript* owner)
{
	_elapsedTime += TIME->DeltaTime();
	if (_elapsedTime >= owner->_fadeOutTime) {
		_elapsedTime = 0.0f;
		// 사망시 FadeOut, 다시 씬을 로드하는 부분 필요
		// owner->SetState(MainSceneState::Common);
	}
}

void MainSceneScript::BossFight::StateStart(MainSceneScript* owner)
{
	RENDER->GetObjectW("Brute")->GetComponent<BossScript>()->blockExecute = false;
	owner->_mainAs->Play();
}

void MainSceneScript::BossFight::StateUpdate(MainSceneScript* owner)
{

}
