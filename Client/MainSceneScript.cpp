#include "pch.h"
#include "MainSceneScript.h"
#include "BossScript.h"
#include "Managers.h"

REGISTER_COMPONENT(MainSceneScript)

MainSceneScript::~MainSceneScript()
{

}

void MainSceneScript::Init()
{
#ifndef BULB_EDITOR
	INPUTM->SetMouseCenterFixMode(true);
	ShowCursor(FALSE);
#endif

	_states.push_back(new FadeIn());
	_states.push_back(new Common());
	_states.push_back(new FadeOut());
	_states.push_back(new BossFight());
	SetState(MainSceneState::FadeIn);

	// info popup
	{
		_infoPanel = UI->CreateUI<UIPanel>();
		_infoPanel->GetTransform()->SetPivot({ 0.5f, 0.5f });
		_infoPanel->GetTransform()->SetPosition({ 0.0f, -200.0f, 0.0f });
		_infoPanel->GetTransform()->SetSize({ 800.0f, 250.0f });
		_infoPanel->SetColor({ 0.0f, 0.0f, 0.0f, 0.3f });
		_infoPanel->SetRenderActive(false);

		_infoText = UI->CreateUI<UIText>();
		_infoText->GetTransform()->SetSize({ 480.0f, 280.0f });
		_infoText->SetFont(L"KoPubBatang");
		_infoText->SetFontSize(32);
		_infoText->SetRenderActive(false);
		_infoText->GetTransform()->SetParent(_infoPanel->GetTransform());
	}

	// Boss info ui(hp bar, boss name text)
	{
		_bossInfoPanel = UI->CreateUI<UIPanel>();
		_bossInfoPanel->SetColor({ 0.0f, 0.0f, 0.0f, 0.0f });
		_bossInfoPanel->GetTransform()->SetPivot({ 0.5f, 0.0f });
		_bossInfoPanel->GetTransform()->SetPosition({ 0.0f, -400.0f, 0.0f });
		_bossInfoPanel->GetTransform()->SetSize({ 800.0f, 40.0f });
		_bossInfoPanel->SetRenderActive(false);

		_bossHpBar = UI->CreateUI<UISlider>();
		_bossHpBar->GetTransform()->SetParent(_bossInfoPanel->GetTransform());
		_bossHpBar->SetEntireSize({ 800.0f, 10.0f });
		_bossHpBar->GetTransform()->SetPivot({ 0.5f, 0.0f });
		_bossHpBar->GetTransform()->SetLocalPosition({ 0.0f, -20.0f, 0.0f });
		_bossHpBar->SetFillColor({ 1.0f, 0.0f, 0.0f, 1.0f });
		_bossHpBar->SetValueMaxLimit(100.0f);
		_bossHpBar->SetValue(100.0f);
		_bossHpBar->SetRenderActive(false);

		_bossNameText = UI->CreateUI<UIText>();
		_bossNameText->GetTransform()->SetParent(_bossInfoPanel->GetTransform());
		_bossNameText->GetTransform()->SetPivot({ 0.0f, 1.0f });
		_bossNameText->GetTransform()->SetLocalPosition({ -400.0f, 20.0f, 0.0f });
		_bossNameText->SetSize({ 800.0f, 30.0f });
		_bossNameText->SetFont(L"KoPubBatang");
		_bossNameText->SetFontSize(24);
		_bossNameText->SetText(L"옛 왕");
		_bossNameText->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		_bossNameText->SetRenderActive(false);
	}

	// pause menu
	{
		_pauseUI = UI->CreateUI<UIFrame>();
		_pauseUI->GetTransform()->SetStretchSize(true);
		_pauseUI->GetTransform()->SetSizePercentage({ 0.8f, 0.8f });

		_pauseBackgroundPanel = UI->CreateUI<UIPanel>();
		_pauseBackgroundPanel->SetColor({ 0.2f, 0.2f, 0.2f, 0.5f });
		_pauseBackgroundPanel->GetTransform()->SetParent(_pauseUI->GetTransform());
		_pauseBackgroundPanel->GetTransform()->SetStretchSize(true);
		_pauseBackgroundPanel->GetTransform()->SetSizePercentage({ 1.0f, 1.0f });

		//shared_ptr<UIText> titleText = UI->CreateUI<UIText>();
		//titleText->GetTransform()->SetParent(_pauseBackgroundPanel->GetTransform());
		//titleText->GetTransform()->SetPivot({ 0.5f, 1.0f });
		//titleText->GetTransform()->SetAnchor(UIAnchorMode::CenterTop);
		//titleText->GetTransform()->SetLocalPosition({ 0.0f, -20.0f, 0.0f });
		//titleText->SetSize({ 800.0f, 50.0f });
		//titleText->SetFont(L"KoPubBatang");
		//titleText->SetFontSize(42);
		//titleText->SetText(L"일시정지");
		//titleText->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

		_closeBtn = UI->CreateUI<UIButton>();
		_closeBtn->GetTransform()->SetParent(_pauseBackgroundPanel->GetTransform());
		_closeBtn->GetTransform()->SetPivot({ 0.0f, 1.0f });
		_closeBtn->GetTransform()->SetAnchor(UIAnchorMode::LeftTop);
		_closeBtn->GetTransform()->SetLocalPosition({ 0.0f, -180.0f, -1.0f });
		_closeBtn->GetTransform()->SetSize({ 300.0f, 60.0f });
		_closeBtn->text->SetFont("Georgia");
		_closeBtn->text->SetText("계속하기");

		_settingBtn = UI->CreateUI<UIButton>();
		_settingBtn->GetTransform()->SetParent(_pauseBackgroundPanel->GetTransform());
		_settingBtn->GetTransform()->SetPivot({ 0.0f, 1.0f });
		_settingBtn->GetTransform()->SetAnchor(UIAnchorMode::LeftTop);
		_settingBtn->GetTransform()->SetLocalPosition({ 0.0f, -260.0f, -1.0f });
		_settingBtn->GetTransform()->SetSize({ 300.0f, 60.0f });
		_settingBtn->text->SetFont("Georgia");
		_settingBtn->text->SetText("환경설정");

		_exitBtn = UI->CreateUI<UIButton>();
		_exitBtn->GetTransform()->SetParent(_pauseBackgroundPanel->GetTransform());
		_exitBtn->GetTransform()->SetPivot({ 0.0f, 1.0f });
		_exitBtn->GetTransform()->SetAnchor(UIAnchorMode::LeftTop);
		_exitBtn->GetTransform()->SetLocalPosition({ 0.0f, -340.0f, -1.0f });
		_exitBtn->GetTransform()->SetSize({ 300.0f, 60.0f });
		_exitBtn->text->SetFont("Georgia");
		_exitBtn->text->SetText("메인메뉴로 나가기");
		// _closeBtn->mouseUpEvent += [this]() { OnClickedLevelUpButton(); };

		_pauseUI->SetRenderActive(false);
	}

	_sndBoss = SOUND->LoadSound("Sounds/Boss.mp3", false);
	_sndBossLoop = SOUND->LoadSound("Sounds/BossLoop.mp3", true);

	_mainAs = GetGameObject()->GetComponent<AudioSource>();

	_mainAs->SetSound(_sndBoss);
	_mainAs->SetLoop(true);
	_mainAs->SetAsBGM(true);

	Managers::Ui.pauseUIEvent += [this]() {
		this->ActivatePauseUI();
	};
}

void MainSceneScript::Update()
{
	if (_isStateChanged) {
		_states[static_cast<int>(_currentState)]->StateStart(this);
		_isStateChanged = false;
	}

	_states[static_cast<int>(_currentState)]->StateUpdate(this);

	if (INPUTM->IsKeyDown(KeyValue::E) && _isInfoPanelActive) {
		_infoPanel->SetRenderActive(false);
		_isInfoPanelActive = false;
	}
}

void MainSceneScript::OnDestroy()
{
	_infoPanel.reset();
	_infoText.reset();
	_bossInfoPanel.reset();
	_bossHpBar.reset();
	_bossNameText.reset();

	_mainAs.reset();

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

void MainSceneScript::SetInfoPanel(wstring content)
{
	_infoText->SetText(content);
	_infoPanel->SetRenderActive(true);
	_isInfoPanelActive = true;
}

void MainSceneScript::ActivatePauseUI()
{
	Managers::Ui.AddUI(_pauseUI);
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
	_boss = RENDER->GetObjectW("Brute")->GetComponent<BossScript>();
	_boss->blockExecute = false;
	owner->_bossHpBar->SetValueMaxLimit(_boss->GetCurrentHealth());
	owner->_bossInfoPanel->SetRenderActive(true);
	owner->_mainAs->Play();
}

void MainSceneScript::BossFight::StateUpdate(MainSceneScript* owner)
{
	if (_boss != nullptr) {
		int h = _boss->GetCurrentHealth();
		owner->_bossHpBar->SetValue(h);
		if (h <= 0)
			_boss.reset();
	}
}
