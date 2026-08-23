#pragma once
#include "Script.h"
#include "BaseState.h"

class StartMenuSceneScript : public Script
{
	enum class StartMenuSceneState {
		Init = -1,
		MenuFadeIn,
		Menu,
		MenuFadeOut
	};

	class MenuFadeIn : public BaseState<StartMenuSceneScript> {
		void StateStart(StartMenuSceneScript* owner) override;
		void StateUpdate(StartMenuSceneScript* owner) override;

	private:
		float _elapsedTime = 0.0f;
	};

	class Menu : public BaseState<StartMenuSceneScript> {
		void StateStart(StartMenuSceneScript* owner) override;
	};

	class MenuFadeOut : public BaseState<StartMenuSceneScript> {
		void StateStart(StartMenuSceneScript* owner) override;
		void StateUpdate(StartMenuSceneScript* owner) override;

	private:
		float _elapsedTime = 0.0f;
	};

public:
	~StartMenuSceneScript();

	void Init() override;
	void Update() override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;

	void SaveXML(Bulb::XMLElement compElem) override;

	ComponentSnapshot CaptureSnapshot() override;
	void RestoreSnapshot(ComponentSnapshot snapshot) override;

public:
	void OnMouseEnterButton();

	void OnClickedStartButton();
	void OnClickedSettingsButton();
	void OnClickedExitButton();

private:
	void SetState(StartMenuSceneState state) {
		if (_currentState == state) return;
		_currentState = state;
		_isStateChanged = true;
	}

private:
	vector<BaseState<StartMenuSceneScript>*> _states;
	StartMenuSceneState _currentState = StartMenuSceneState::Init;
	bool _isStateChanged = false;

	shared_ptr<UIButton> _startButton;
	shared_ptr<UIButton> _settingButton;
	shared_ptr<UIButton> _exitButton;

	float _fadeInTime = 1.0f;
	float _soundFadeOutTime = 2.0f;

	AudioClip _sndMainTheme;
	AudioClip _sndBtnHover;
	AudioClip _sndGameStart;

	shared_ptr<AudioSource> _asMainTheme;
};
