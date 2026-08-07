#pragma once
#include "Script.h"
#include "BaseState.h"

enum class MainSceneState {
	Init = -1,
	FadeIn,
	Common,
	FadeOut,
	BossFight
};

class MainSceneScript : public Script
{

	class FadeIn : public BaseState<MainSceneScript> {
		void StateStart(MainSceneScript* owner) override;
		void StateUpdate(MainSceneScript* owner) override;

	private:
		float _elapsedTime = 0.0f;
	};

	class Common : public BaseState<MainSceneScript> {

	};

	class FadeOut : public BaseState<MainSceneScript> {
		void StateStart(MainSceneScript* owner) override;
		void StateUpdate(MainSceneScript* owner) override;

	private:
		float _elapsedTime = 0.0f;
	};

	class BossFight : public BaseState<MainSceneScript> {
		void StateStart(MainSceneScript* owner) override;
		void StateUpdate(MainSceneScript* owner) override;
	};

public:
	~MainSceneScript();

	void Init() override;
	void Update() override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;

	void SaveXML(Bulb::XMLElement compElem) override;

	ComponentSnapshot CaptureSnapshot() override;

	void RestoreSnapshot(ComponentSnapshot snapshot) override;

	void SetState(MainSceneState state) {
		if (_currentState == state) return;
		_currentState = state;
		_isStateChanged = true;
	}

private:
	vector<BaseState<MainSceneScript>*> _states;
	MainSceneState _currentState = MainSceneState::Init;
	bool _isStateChanged = false;

	float _fadeInTime = 1.0f;
	float _fadeOutTime = 1.0f;
};

