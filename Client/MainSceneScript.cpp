#include "pch.h"
#include "MainSceneScript.h"
#include "BossScript.h"

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

	// Boss info ui(hp bar, boss name text)
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

	_sndBoss = SOUND->LoadSound("Sounds/Boss.mp3", false);
	_sndBossLoop = SOUND->LoadSound("Sounds/BossLoop.mp3", true);

	_mainAs = GetGameObject()->GetComponent<AudioSource>();

	_mainAs->SetSound(_sndBoss);
	_mainAs->SetLoop(true);
	_mainAs->SetAsBGM(true);
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
