#include "pch.h"
#include "PlayerScript.h"
#include "TPVCamera.h"
#include "EnemyScript.h"
#include "Interactable.h"
#include "MainSceneScript.h"

using namespace Bulb;

REGISTER_COMPONENT(PlayerScript)

PlayerScript::~PlayerScript()
{
	cout << "p";

	for (auto state : _states)
		delete state;
}

void PlayerScript::Init()
{
	_gameObject = GetGameObject();

	_transform = _gameObject->GetTransform();

	auto swordObj = _transform->GetChild("mixamorig:Sword_joint")->GetGameObject();
	swordObj->SetTag("AttackAlly");

	_swordRb = static_pointer_cast<Rigidbody>(ComponentFactory::Create("Rigidbody"));
	_swordRb->SetStatic(true);
	_swordRb->SetGravity(false);
	_swordRb->SetColliderTrigger(true);
	_swordRb->SetColliderExtents({ 0.035f, 0.02f, 0.37f });
	_swordRb->SetColliderOffset({ 0.0f, 0.0f, -0.46f });
	_swordRb->SetColliderRotationOffset({ -4.0f, 15.0f, 0.0f });
	_swordRb->SetPhysicsActive(false);
	swordObj->AddComponent(_swordRb);

	_swordAs = swordObj->GetComponent<AudioSource>();

	_animator = _gameObject->GetComponent<Animator>();
	_animator->LoadAnimationEvents("..\\Resources\\Animations\\Paladin WProp J Nordstrom\\AnimationEvents.xml");
	_animator->animationEvent += [this](AnimationEvent e) {
		this->AnimationEventListener(e);
	};

	_animator->AddAnimation(RESOURCE->LoadAnimation("Paladin WProp J Nordstrom\\walk_sword_back"));
	_animator->AddAnimation(RESOURCE->LoadAnimation("Paladin WProp J Nordstrom\\strafe_sword_left"));
	_animator->AddAnimation(RESOURCE->LoadAnimation("Paladin WProp J Nordstrom\\strafe_sword_left_run"));
	_animator->AddAnimation(RESOURCE->LoadAnimation("Paladin WProp J Nordstrom\\strafe_sword_right"));
	_animator->AddAnimation(RESOURCE->LoadAnimation("Paladin WProp J Nordstrom\\strafe_sword_right_run"));

	_playerMovementState = PlayerMovementState::IDLE;
	_isStateChanged = true;

	_movingDirection = { 0.0f, 0.0f, 0.0f };

	_controller = _gameObject->GetComponent<CharacterController>();
	_controller->SetHalfHeight(0.5f);
	_controller->SetRadius(0.3f);
	_controller->SetOffset(Vector3(0.0f, 0.8f, 0.0f));

	_hpBar = UI->CreateUI<UISlider>();
	_hpBar->GetTransform()->SetPivot({ 0.0f, 1.0f });
	_hpBar->GetTransform()->SetPosition({ -800.0f, 450.0f, 0.0f });
	_hpBar->SetFillColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	_hpBar->SetValueMaxLimit(100.0f);
	_hpBar->SetValue(100.0f);

	_steminaBar = UI->CreateUI<UISlider>();
	_steminaBar->GetTransform()->SetPivot({ 0.0f, 1.0f });
	_steminaBar->GetTransform()->SetPosition({ -800.0f, 420.0f, 0.0f });
	_steminaBar->SetFillColor({ 0.0f, 1.0f, 0.0f, 1.0f });
	_steminaBar->SetValueMaxLimit(steminaMax);
	_steminaBar->SetValue(stemina);

	tpvCameraScript = RENDER->GetObject("TPVCamera")->GetComponent<TPVCamera>();

	_states.push_back(new IdleState());
	_states.push_back(new WalkState());
	_states.push_back(new RunState());
	_states.push_back(new SlashState());
	_states.push_back(new RollState());
	_states.push_back(new StrafeForwardState());
	_states.push_back(new StrafeBackState());
	_states.push_back(new StrafeRightState());
	_states.push_back(new StrafeLeftState());
	_states.push_back(new InteractState());
	_states.push_back(new EnterVeilState());

	_playerFootstepSounds[0] = SOUND->LoadSound("Sounds/PlayerStep1.wav", false);
	_playerFootstepSounds[1] = SOUND->LoadSound("Sounds/PlayerStep2.wav", false);
	_playerFootstepSounds[2] = SOUND->LoadSound("Sounds/PlayerStep3.wav", false);
	_playerFootstepSounds[3] = SOUND->LoadSound("Sounds/PlayerStep4.wav", false);
	_playerFootstepSounds[4] = SOUND->LoadSound("Sounds/PlayerStep5.wav", false);

	_playerFootAs = _gameObject->GetComponent<AudioSource>();
}

void PlayerScript::Update()
{
	if (INPUTM->IsKeyDown(KeyValue::Q))
		LockOn();

	RecoveryStemina();

	Roll();
	Attack();

	// Interact
	Interact();

	if (_playerMovementState == PlayerMovementState::SLASH || 
		_playerMovementState == PlayerMovementState::ROLL ||
		_playerMovementState == PlayerMovementState::INTERACT ||
		_playerMovementState == PlayerMovementState::ENTER_VEIL)
	{
		if (_animator->IsCurrentAnimationEnd())
		{
			DEBUG->Log("Idle Set");
			SetState(PlayerMovementState::IDLE);
		}
	}
	else if (_playerMovementState != PlayerMovementState::ROLL)
	{
		Move();
	}

	if (_isStateChanged)
	{
		_states[static_cast<int>(_playerMovementState)]->StateStart(this);
		_isStateChanged = false;
	}

	_states[static_cast<int>(_playerMovementState)]->StateUpdate(this);
}

void PlayerScript::OnCollisionEnter(shared_ptr<GameObject> other)
{
	if (other->GetTag() == "AttackHostile")
	{
		if (health > 0 && !_isEvading) {
			TakeDamage(other->GetComponent<Rigidbody>()->customData);
		}
	}

	if (other->GetTag() == "Interactable") {
		_interactableScripts.push_back(other->GetComponent<Interactable>());
	}
}

void PlayerScript::OnCollisionExit(shared_ptr<GameObject> other)
{
	if (other->GetTag() == "Interactable") {
		for (int i = 0; i < _interactableScripts.size(); ++i) {
			if (other == _interactableScripts[i]->GetGameObject()) {
				_interactableScripts.erase(_interactableScripts.begin() + i);
				break;
			}
		}
	}
}

void PlayerScript::OnDestroy()
{
	cout << "OnDestroy - PlayerScript:" << _id << "\n";

	tpvCameraScript.reset();
	_gameObject.reset();
	_transform.reset();
	_animator.reset();
	_controller.reset();
	_hpBar.reset();
	_steminaBar.reset();
	_lockOnTarget.reset();
}

void PlayerScript::LoadXML(Bulb::XMLElement compElem)
{

}

void PlayerScript::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "PlayerScript");
}

void PlayerScript::Roll()
{
	if (INPUTM->IsKeyDown(KeyValue::SPACE) && 
		!_animator->IsTransitionBlocked() && 
		_playerMovementState != PlayerMovementState::IDLE) {
		SetState(PlayerMovementState::ROLL);
	}
}

void PlayerScript::Attack()
{
	if (INPUTM->IsMouseLeftButtonDown() && 
		!_animator->IsTransitionBlocked() && 
		_playerMovementState != PlayerMovementState::SLASH) {
		SetState(PlayerMovementState::SLASH);
	}
}

void PlayerScript::Move()
{
	if (!_controller->IsOnGround()) return;

	int moveZ = (INPUTM->IsKeyPress(KeyValue::W) ? 1 : 0) - (INPUTM->IsKeyPress(KeyValue::S) ? 1 : 0);
	int moveX = (INPUTM->IsKeyPress(KeyValue::D) ? 1 : 0) - (INPUTM->IsKeyPress(KeyValue::A) ? 1 : 0);

	if (moveZ == 0 && moveX == 0) {
		SetState(PlayerMovementState::IDLE);
		return;
	}

	shared_ptr<Transform> cameraTransform = Camera::GetCurrentCamera()->GetTransform();
	Vector3 look = cameraTransform->GetLook();
	Vector3 right = cameraTransform->GetRight();
	look.y = 0.0f;
	right.y = 0.0f;

	_movingDirection = (look * moveZ + right * moveX).Normalize();
	Bulb::Vector3 interpolatedDir = MathHelper::InterpolateVector(_transform->GetBack(), _movingDirection, _rotationSpeed);

	if ((INPUTM->IsKeyDown(KeyValue::SHIFT) || _playerMovementState == PlayerMovementState::RUN) &&
		stemina > 0.0f) {
		_movingDirection = interpolatedDir;
		SetState(PlayerMovementState::RUN);
	}
	else {
		if (_lockOnTarget) {
			if (moveX == 1)
				SetState(PlayerMovementState::STRAFE_RIGHT);
			else if (moveX == -1)
				SetState(PlayerMovementState::STRAFE_LEFT);
			else if (moveZ == 1)
				SetState(PlayerMovementState::STRAFE_FORWARD);
			else if (moveZ == -1)
				SetState(PlayerMovementState::STRAFE_BACK);
		}
		else {
			_movingDirection = interpolatedDir;
			SetState(PlayerMovementState::WALK);
		}
	}
}

void PlayerScript::LockOn()
{
	if (_isLockOn) {
		_isLockOn = false;
		_lockOnTarget = nullptr;

		if (tpvCameraScript == nullptr)
			DEBUG->ErrorLog("Can't Find TPVCamera Component!");
		else {
			tpvCameraScript->isLockOn = false;
			tpvCameraScript->lockOnTargetTransform = nullptr;
		}
		DEBUG->Log("Locked On Target Released");
		return;
	}

	vector<shared_ptr<GameObject>> enemies = PHYSICS->OverlapSphere(_transform->GetPosition(), 20.0f, "Enemy"); float minDistance = FLT_MAX;
	shared_ptr<GameObject> bestTarget = nullptr;

	Vector3 camPos = Camera::GetCurrentCamera()->GetTransform()->GetPosition();
	Vector3 camLook = Camera::GetCurrentCamera()->GetTransform()->GetLook();

	for (auto& enemy : enemies) {
		Vector3 enemyPos = enemy->GetTransform()->GetPosition();
		Vector3 dirToEnemy = (enemyPos - camPos).Normalize();

		// 시야각 체크 (내적)
		float dot = camLook.Dot(dirToEnemy);
		if (dot < 0.5f) continue; // 실제 사용중인 pov값 받아오도록 바꿔야함.

		// 레이캐스트 장애물 체크
		//RaycastHit hit;
		//if (PHYSICS->Raycast(camPos, dirToEnemy, 15.0f, Layer::Wall)) continue;

		// 가장 가까운 거리 비교
		float dist = (_transform->GetPosition() - enemyPos).Length();
		if (dist < minDistance) {
			minDistance = dist;
			bestTarget = enemy;
		}
	}

	if (bestTarget) {
		_lockOnTarget = bestTarget;
		_isLockOn = true;

		if (tpvCameraScript == nullptr)
			DEBUG->ErrorLog("Can't Find TPVCamera Component!");
		else {
			tpvCameraScript->isLockOn = true;
			tpvCameraScript->lockOnTargetTransform = _lockOnTarget->GetComponent<EnemyScript>()->GetCenterTransform();
		}

		DEBUG->Log("Locked On - " + _lockOnTarget->GetName());
	}
}

void PlayerScript::Interact()
{
	// 1. 상호작용 비활성화 객체 삭제
	// 2. 삭제 후 상호작용 검증
	// + 따라서 원래 비활성화 상태였던 객체가 활성화 상태로 바뀐 경우 상호작용을 하려면 다시 CollisionEnter가 작동해야함
	for (int i = 0; i < _interactableScripts.size(); ++i) {
		if (!_interactableScripts[i]->isInteractable) {
			_interactableScripts.erase(_interactableScripts.begin() + i);
			--i;
		}
	}

	if (_interactableScripts.size() > 0) {
		if (INPUTM->IsKeyDown(KeyValue::E) &&
			!_animator->IsTransitionBlocked()) {
			_interactableScripts[0]->Interact(_gameObject);
		}
	}
}

void PlayerScript::RecoveryStemina()
{
	if (!_isRecoveryPossible)
		_recoverySteminaDelayedTime -= TIME->DeltaTime();
	else if (stemina < steminaMax) {
		stemina += 10.0f * TIME->DeltaTime();
		if (stemina > steminaMax) stemina = steminaMax;
	}

	if (_recoverySteminaDelayedTime < 0.0f) {
		_recoverySteminaDelayedTime = 0.0f;
		_isRecoveryPossible = true;
	}

	_steminaBar->SetValue(stemina);
}

void PlayerScript::TakeDamage(int damage)
{
	health -= damage;
	DEBUG->Log("Take" + to_string(damage) + "damage. Remain Health - " + to_string(health));
	_hpBar->SetValue(health);
}

void PlayerScript::DecreaseStemina(float value, bool instantChange)
{
	if (instantChange) {
		stemina -= value;
	}
	else {
		stemina -= value * TIME->DeltaTime();
	}

	if (stemina < 0.0f) stemina = 0.0f;

	_recoverySteminaDelayedTime = 2.0f;
	_isRecoveryPossible = false;
}

void PlayerScript::AnimationEventListener(AnimationEvent event)
{
	if (event.type == AnimationEventTypes::Attack) {
		bool attackFlag = event.datas[2].x == 1;

		_swordRb->SetPhysicsActive(attackFlag);
		_swordRb->customData = event.datas[0].w;

		if (attackFlag)
			_swordAs->Play();
	}

	if (event.type == AnimationEventTypes::Step) {
		_playerFootAs->SetSound(_playerFootstepSounds[Utils::Random(0, 4)]);
		_playerFootAs->Play();
	}

	if (event.type == AnimationEventTypes::Evade) {
		_isEvading = event.datas[0].x == 1;
	}
}

void PlayerScript::IdleState::StateStart(PlayerScript* owner)
{
	owner->_animator->SetCurrentAnimation("idle_sword_4");
	owner->_animator->SetLoop(true);
}

void PlayerScript::IdleState::StateUpdate(PlayerScript* owner)
{
	if (owner->_lockOnTarget) {
		owner->_transform->LookAtWithNoRoll(
			owner->_transform->GetPosition() -
			MathHelper::InterpolateVector(
				owner->_transform->GetBack(),
				owner->_lockOnTarget->GetTransform()->GetPosition() - owner->_transform->GetPosition(),
				owner->_rotationSpeed
			)
		);
	}
}

void PlayerScript::WalkState::StateStart(PlayerScript* owner)
{
	owner->_animator->SetCurrentAnimation("walk_sword_forward");
	owner->_animator->SetLoop(true);
}

void PlayerScript::WalkState::StateUpdate(PlayerScript* owner)
{
	if (!owner->_controller->IsOnGround()) {
		owner->SetState(PlayerMovementState::IDLE);
		return;
	}

	owner->_transform->LookAtWithNoRoll(owner->_transform->GetPosition() - owner->_movingDirection);
	owner->_controller->SetVelocity(owner->_movingDirection * owner->_speed);
}

void PlayerScript::RunState::StateStart(PlayerScript* owner)
{
	owner->_animator->SetCurrentAnimation("run_sword_forward");
	owner->_animator->SetLoop(true);
}

void PlayerScript::RunState::StateUpdate(PlayerScript* owner)
{
	if (!owner->_controller->IsOnGround()) {
		owner->SetState(PlayerMovementState::IDLE);
		return;
	}

	owner->_transform->LookAtWithNoRoll(owner->_transform->GetPosition() - owner->_movingDirection);
	owner->_controller->SetVelocity(owner->_movingDirection * owner->_speed * 3.0f);
	owner->DecreaseStemina(10.0f, false);
}

void PlayerScript::SlashState::StateStart(PlayerScript* owner)
{
	owner->_animator->SetCurrentAnimation("slash_1");
	owner->_animator->SetLoop(false);
}

void PlayerScript::SlashState::StateUpdate(PlayerScript* owner)
{

}

void PlayerScript::RollState::StateStart(PlayerScript* owner)
{
  	owner->_animator->SetCurrentAnimation("roll");
 	owner->_animator->SetLoop(false);
}

void PlayerScript::RollState::StateUpdate(PlayerScript* owner)
{
	owner->_transform->LookAtWithNoRoll(owner->_transform->GetPosition() - owner->_movingDirection);
	owner->_controller->SetVelocity(owner->_movingDirection * owner->_speed * 4.0f);
}

void PlayerScript::StrafeForwardState::StateStart(PlayerScript* owner)
{
	owner->_animator->SetCurrentAnimation("walk_sword_forward");
	owner->_animator->SetLoop(true);
}

void PlayerScript::StrafeForwardState::StateUpdate(PlayerScript* owner)
{
	owner->_transform->LookAtWithNoRoll(
		owner->_transform->GetPosition() -
		MathHelper::InterpolateVector(
			owner->_transform->GetBack(),
			owner->_lockOnTarget->GetTransform()->GetPosition() - owner->_transform->GetPosition(),
			owner->_rotationSpeed
		)
	);
	owner->_controller->SetVelocity(owner->_movingDirection * owner->_speed);
}

void PlayerScript::StrafeBackState::StateStart(PlayerScript* owner)
{
	owner->_animator->SetCurrentAnimation("walk_sword_back");
	owner->_animator->SetLoop(true);
}

void PlayerScript::StrafeBackState::StateUpdate(PlayerScript* owner)
{
	owner->_transform->LookAtWithNoRoll(
		owner->_transform->GetPosition() -
		MathHelper::InterpolateVector(
			owner->_transform->GetBack(),
			owner->_lockOnTarget->GetTransform()->GetPosition() - owner->_transform->GetPosition(),
			owner->_rotationSpeed
		)
	);
	owner->_controller->SetVelocity(owner->_movingDirection * owner->_speed * 0.8f);
}

void PlayerScript::StrafeRightState::StateStart(PlayerScript* owner)
{
	owner->_animator->SetCurrentAnimation("strafe_sword_right");
	owner->_animator->SetLoop(true);
}

void PlayerScript::StrafeRightState::StateUpdate(PlayerScript* owner)
{
	owner->_transform->LookAtWithNoRoll(
		owner->_transform->GetPosition() -
		MathHelper::InterpolateVector(
			owner->_transform->GetBack(),
			owner->_lockOnTarget->GetTransform()->GetPosition() - owner->_transform->GetPosition(),
			owner->_rotationSpeed
		)
	);
	owner->_controller->SetVelocity(owner->_movingDirection * owner->_speed * 0.8f);
}

void PlayerScript::StrafeLeftState::StateStart(PlayerScript* owner)
{
	owner->_animator->SetCurrentAnimation("strafe_sword_left");
	owner->_animator->SetLoop(true);
}

void PlayerScript::StrafeLeftState::StateUpdate(PlayerScript* owner)
{
	owner->_transform->LookAtWithNoRoll(
		owner->_transform->GetPosition() -
		MathHelper::InterpolateVector(
			owner->_transform->GetBack(),
			owner->_lockOnTarget->GetTransform()->GetPosition() - owner->_transform->GetPosition(),
			owner->_rotationSpeed
		)
	);
	owner->_controller->SetVelocity(owner->_movingDirection * owner->_speed * 0.8f);
}

void PlayerScript::InteractState::StateStart(PlayerScript* owner)
{
	owner->_animator->SetCurrentAnimation("casting_sword");
	owner->_animator->SetLoop(false);
}

void PlayerScript::InteractState::StateUpdate(PlayerScript* owner)
{

}

void PlayerScript::EnterVeilState::StateStart(PlayerScript* owner)
{
	owner->_animator->SetCurrentAnimation("walk_sword_forward");
	owner->_animator->SetLoop(true);

	owner->_controller->SetGravity(false);
	owner->_controller->SetPhysicsActive(false);

	owner->_transform->LookAtWithNoRoll(owner->_transform->GetPosition() - Bulb::Vector3(1.0f, 0.0f, 0.0f));
}

void PlayerScript::EnterVeilState::StateUpdate(PlayerScript* owner)
{
	if (owner->_transform->GetPosition().x < 23.5f) {
		owner->_transform->Translate(Bulb::Vector3(1.0f, 0.0f, 0.0f) * owner->_speed * 0.8f * TIME->DeltaTime());
	}
	else {
		owner->SetState(PlayerMovementState::IDLE);
		owner->_controller->SetGravity(true);
		owner->_controller->SetPhysicsActive(true);
		RENDER->GetObjectW("SceneScript")->GetComponent<MainSceneScript>()->SetState(MainSceneState::BossFight);
	}
}
