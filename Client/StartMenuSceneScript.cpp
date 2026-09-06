#include "pch.h"
#include "StartMenuSceneScript.h"

REGISTER_COMPONENT(StartMenuSceneScript)

StartMenuSceneScript::~StartMenuSceneScript()
{

}

void StartMenuSceneScript::Init()
{
	weak_ptr<StartMenuSceneScript> weak = static_pointer_cast<StartMenuSceneScript>(shared_from_this());

	_states.push_back(new MenuFadeIn());
	_states.push_back(new Menu());
	_states.push_back(new MenuFadeOut());
	SetState(StartMenuSceneState::MenuFadeIn);

	_startButton = UI->CreateUI<UIButton>();
	_startButton->GetTransform()->SetDepth(3.0f);
	_startButton->GetTransform()->SetPosition({ 0.0f, -100.0f, 0.0f });
	_startButton->background->SetTexture(L"..\\Resources\\Textures\\UI\\ButtonHovered.png");
	_startButton->background->GetTransform()->SetSize({ 300.0f, 75.0f });
	_startButton->text->SetFont("Georgia");
	_startButton->text->SetText("Start Game");
	_startButton->mouseEnterEvent += [this]() { OnMouseEnterButton(); };
	_startButton->mouseDownEvent += [this]() { OnClickedStartButton(); };

	_settingButton = UI->CreateUI<UIButton>();
	_settingButton->GetTransform()->SetDepth(3.0f);
	_settingButton->GetTransform()->SetPosition({ 0.0f, -175.0f, 0.0f });
	_settingButton->background->SetTexture(L"..\\Resources\\Textures\\UI\\ButtonHovered.png");
	_settingButton->background->GetTransform()->SetSize({ 300.0f, 75.0f });
	_settingButton->text->SetFont("Georgia");
	_settingButton->text->SetText("Settings");
	_settingButton->mouseEnterEvent += [this]() { OnMouseEnterButton(); };
	_settingButton->mouseDownEvent += [this]() { OnClickedSettingsButton(); };

	_exitButton = UI->CreateUI<UIButton>();
	_exitButton->GetTransform()->SetDepth(3.0f);
	_exitButton->GetTransform()->SetPosition({ 0.0f, -250.0f, 0.0f });
	_exitButton->background->SetTexture(L"..\\Resources\\Textures\\UI\\ButtonHovered.png");
	_exitButton->background->GetTransform()->SetSize({ 300.0f, 75.0f });
	_exitButton->text->SetFont("Georgia");
	_exitButton->text->SetText("Exit");
	_exitButton->mouseEnterEvent += [this]() { OnMouseEnterButton(); };
	_exitButton->mouseDownEvent += [this]() { OnClickedExitButton(); };

	shared_ptr<UIPanel> mainTitlePanel = UI->CreateUI<UIPanel>();
	mainTitlePanel->SetTexture(L"..\\Resources\\Textures\\Logos\\MainTitle.png");
	mainTitlePanel->GetTransform()->SetDepth(5.0f);
	mainTitlePanel->GetTransform()->SetSize({ 1500.0f, 468.75f });
	mainTitlePanel->GetTransform()->SetPosition({ 0.0f, 150.0f, 0.0f });

	_sndMainTheme = SOUND->LoadSound("Sounds/MainMenu.mp3", true);
	_sndBtnHover = SOUND->LoadSound("Sounds/UIButtonHovered.mp3", false);
	_sndGameStart = SOUND->LoadSound("Sounds/StartGameButton.mp3", false);

	_asMainTheme = GetGameObject()->GetComponent<AudioSource>();
	_asMainTheme->SetSound(_sndMainTheme);

	SOUND->SetMasterVolume(0.6f);

	SOUND->AddGroup("BGM");
}

void StartMenuSceneScript::Update()
{
	if (_isStateChanged) {
		_states[static_cast<int>(_currentState)]->StateStart(this);
		_isStateChanged = false;
	}

	_states[static_cast<int>(_currentState)]->StateUpdate(this);
}

void StartMenuSceneScript::OnDestroy()
{
	for (int i = 0; i < _states.size(); ++i) {
		delete _states[i];
	}

	_startButton.reset();
	_exitButton.reset();
	_settingButton.reset();

	_asMainTheme.reset();

	_sndMainTheme->release();
	_sndBtnHover->release();
	_sndGameStart->release();
}

void StartMenuSceneScript::LoadXML(Bulb::XMLElement compElem)
{

}

void StartMenuSceneScript::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "StartMenuSceneScript");
}

ComponentSnapshot StartMenuSceneScript::CaptureSnapshot()
{
	ComponentSnapshot snapshot;
	snapshot.id = _id;
	snapshot.componentType = "StartMenuSceneScript";

	return snapshot;
}

void StartMenuSceneScript::RestoreSnapshot(ComponentSnapshot snapshot)
{
	SetState(StartMenuSceneState::MenuFadeIn);
	Camera::GetCurrentCamera()->SetColorBlend({ 0.0f, 0.0f, 0.0f, 1.0f });
}

void StartMenuSceneScript::OnMouseEnterButton()
{
	SOUND->PlaySound(_sndBtnHover);
}

void StartMenuSceneScript::OnClickedStartButton()
{
	SOUND->PlaySound(_sndGameStart);
	SetState(StartMenuSceneState::MenuFadeOut);
}

void StartMenuSceneScript::OnClickedSettingsButton()
{
	SOUND->PlaySound(_sndBtnHover);
}

void StartMenuSceneScript::OnClickedExitButton()
{
	SOUND->PlaySound(_sndBtnHover);
	APP->QuitApplication();
}

void StartMenuSceneScript::MenuFadeIn::StateStart(StartMenuSceneScript* owner)
{
	Camera::GetCurrentCamera()->SetColorBlend({ 0.0f, 0.0f, 0.0f, 1.0f });
	Camera::GetCurrentCamera()->SetColorBlend({ 0.0f, 0.0f, 0.0f, 0.0f }, owner->_fadeInTime);
	_elapsedTime = 0.0f;
}

void StartMenuSceneScript::MenuFadeIn::StateUpdate(StartMenuSceneScript* owner)
{
	_elapsedTime += TIME->DeltaTime();
	if (_elapsedTime >= owner->_fadeInTime) {
		_elapsedTime = 0.0f;
		owner->SetState(StartMenuSceneState::Menu);
	}
}

void StartMenuSceneScript::Menu::StateStart(StartMenuSceneScript* owner)
{
	owner->_asMainTheme->Play();
	// SOUND->PlaySound(owner->_sndMainTheme, nullptr, "BGM");
}

void StartMenuSceneScript::MenuFadeOut::StateStart(StartMenuSceneScript* owner)
{
	_elapsedTime = 0.0f;
	Camera::GetCurrentCamera()->SetColorBlend({ 0.0f, 0.0f, 0.0f, 1.0f });
}

void StartMenuSceneScript::MenuFadeOut::StateUpdate(StartMenuSceneScript* owner)
{
	_elapsedTime += TIME->DeltaTime();
	if (_elapsedTime >= owner->_soundFadeOutTime) {
		// SOUND->StopSoundGroup("BGM");
		owner->_asMainTheme->Stop();
		SCENE->LoadSceneOnRender("MainScene.xml");
	}
	else {
		// SOUND->SetVolume(1.0f - _elapsedTime / owner->_soundFadeOutTime, "BGM");
		owner->_asMainTheme->SetVolume(1.0f - _elapsedTime / owner->_soundFadeOutTime);
	}
}
