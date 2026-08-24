#include "pch.h"
#include "BossScript.h"
#include "PlayerScript.h"
#include "CommonStructs.h"

using namespace Bulb;

REGISTER_COMPONENT(BossScript)

BossScript::~BossScript()
{
	for (auto pattern : _patterns) {
		delete pattern;
	}
}

void BossScript::Init()
{
	_gameObject = GetGameObject();
	_gameObject->SetTag("Enemy");

	_transform = _gameObject->GetTransform();
	_centerTransform = _transform->GetChild("mixamorig:Spine1");	// Mixamo Default Rig Name

	auto axeObj = _transform->GetChild("mixamorig:Weapon")->GetGameObject();
	axeObj->SetTag("AttackHostile");

	_axeRb = static_pointer_cast<Rigidbody>(ComponentFactory::Create("Rigidbody"));
	_axeRb->SetStatic(true);
	_axeRb->SetGravity(false);
	_axeRb->SetColliderTrigger(true);
	_axeRb->SetColliderExtents({ 0.25f, 0.05f, 0.35f });
	_axeRb->SetColliderOffset({ -0.07f, 0.0f, -0.83f });
	_axeRb->SetColliderRotationOffset({ 3.0f, 0.0f, 0.0f });
	_axeRb->SetPhysicsActive(false);
	axeObj->AddComponent(_axeRb);

	_animator = _gameObject->GetComponent<Animator>();
	_animator->LoadAnimationEvents("..\\Resources\\Animations\\Brute\\AnimationEvents.xml");
	_animator->animationEvent += [this](AnimationEvent e) {
		this->AnimationEventListener(e);
	};

	_controller = _gameObject->GetComponent<CharacterController>();
	_controller->SetHalfHeight(0.6f);
	_controller->SetRadius(0.3f);
	_controller->SetOffset(Vector3(0.0f, 0.9f, 0.0f));

	_hitbox = _gameObject->GetComponent<Rigidbody>();
	_hitbox->SetColliderShape(ColliderShape::Capsule);
	_hitbox->SetColliderHalfHeight(0.6f);
	_hitbox->SetColliderRadius(0.3f);
	_hitbox->SetColliderOffset(Vector3(0.0f, 0.9f, 0.0f));
	_hitbox->SetColliderTrigger(true);
	_hitbox->SetStatic(true);
	_hitbox->SetGravity(false);

	_patterns.push_back(new IdleState());
	_patterns.push_back(new TrackWalkState());
	_patterns.push_back(new AttackState());
	_patterns.push_back(new DeathState());

	_enemyStateUI = UI->CreateUI<UIFrame>();
	_enemyStateUI->GetTransform()->SetDynamicPosition(true);
	_enemyStateUI->GetTransform()->SetSize({ 200.0f, 30.0f });

	_damageText = UI->CreateUI<UIText>();
	_damageText->GetTransform()->SetParent(_enemyStateUI->GetTransform());
	_damageText->GetTransform()->SetSize({ 200.0f, 20.0f });
	_damageText->GetTransform()->SetPivot({ 0.5f, 0.0f });
	_damageText->GetTransform()->SetLocalPosition({ 0.0f, -5.0f, 0.0f });
	_damageText->SetFont(L"KoPubBatang");
	_damageText->SetText(L"0");
	_damageText->SetFontSize(22);
	_damageText->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);

	_healthBarUI = UI->CreateUI<UISlider>();
	_healthBarUI->GetTransform()->SetParent(_enemyStateUI->GetTransform());
	_healthBarUI->SetEntireSize({ 200.0f, 10.0f });
	_healthBarUI->GetTransform()->SetPivot({ 0.5f, 1.0f });
	_healthBarUI->GetTransform()->SetLocalPosition({ 0.0f, -5.0f, 0.0f });
	_healthBarUI->SetFillColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	_healthBarUI->SetValueMaxLimit(100.0f);
	_healthBarUI->SetValueMinLimit(0.0f);
	_healthBarUI->SetValue(_health);

	_enemyStateUI->SetRenderActive(false);

	target = RENDER->GetObjectWithTag("Player");

	// Sound Settings
	_footAs = _gameObject->GetComponent<AudioSource>();
	_bodyAs = _centerTransform->GetGameObject()->GetComponent<AudioSource>();
	_axeAs = axeObj->GetComponent<AudioSource>();

	_footstepSounds[0] = SOUND->LoadSound("Sounds/DirtWalk1.wav", false);
	_footstepSounds[1] = SOUND->LoadSound("Sounds/DirtWalk2.wav", false);
	_footstepSounds[2] = SOUND->LoadSound("Sounds/DirtWalk3.wav", false);
	_footstepSounds[3] = SOUND->LoadSound("Sounds/DirtWalk4.wav", false);
	_footstepSounds[4] = SOUND->LoadSound("Sounds/DirtWalk5.wav", false);

	_hitSound = SOUND->LoadSound("Sounds/SwordHit1.wav", false);

	_axeSound = SOUND->LoadSound("Sounds/Axe.wav", false);

	_bodyAs->SetSound(_hitSound);
	_axeAs->SetSound(_axeSound);
}

void BossScript::Update()
{
	if (blockExecute)
		return;

	if (_isOnHitDelay) {
		_hitDelayTime -= TIME->DeltaTime();
		if (_hitDelayTime <= 0.0f) {
			_hitDelayTime = 0.0f;
			_animator->PlayAnimation();
			_isOnHitDelay = false;
		}
	}

	UpdateDamageText();

	_targetVec = target->GetTransform()->GetPosition() - _transform->GetPosition();
	_targetVec.y = 0;
	_targetDistance = _targetVec.Length();

	if (_currentState != BossState::DEATH) {
		if (_health <= 0) {
			SetState(BossState::DEATH);
		}
	}

	if (_isStateChanged) {
		_patterns[static_cast<int>(_currentState)]->StateStart(this);
		_isStateChanged = false;
	}

	_patterns[static_cast<int>(_currentState)]->StateUpdate(this);

	Vector3 pos = _transform->GetPosition();
	_enemyStateUI->GetTransform()->SetPosition({ pos.x, pos.y + 2.2f, pos.z });
}

void BossScript::OnCollisionEnter(shared_ptr<GameObject> other)
{
	if (other->GetTag() == "AttackAlly") {
		if (_health > 0) {
			AttackInfo* attackInfo = (AttackInfo*)(other->GetComponent<Rigidbody>()->customData);
			TakeDamage(attackInfo->damage);
			target->GetComponent<PlayerScript>()->HitDelay();
		}
	}
}

void BossScript::OnDestroy()
{
	cout << "OnDestroy - EnemyScript:" << _id << "\n";

	target.reset();
	_gameObject.reset();
	_transform.reset();
	_animator.reset();
	_controller.reset();
	_hitbox.reset();
	_damageText.reset();
}

void BossScript::LoadXML(Bulb::XMLElement compElem)
{

}

void BossScript::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "EnemyScript");
}

void BossScript::TakeDamage(int damage)
{
	_health -= damage;
	DEBUG->Log("Take" + to_string(damage) + "damage. Remain Health - " + to_string(_health));
	SetDamageText(damage);
	_healthBarUI->SetValue(_health);
	_bodyAs->Play();
}

void BossScript::HitDelay()
{
	_isOnHitDelay = true;
	_hitDelayTime = 0.08f;
	_animator->PauseAnimation();
}

void BossScript::SetDamageText(int damage)
{
	_damageTextTime = 2.0f;
	_cumulativeDamage += damage;
	_damageText->SetText(to_wstring(_cumulativeDamage));

	if (!_enemyStateUI->IsRenderActive())
		_enemyStateUI->SetRenderActive(true);
}

void BossScript::UpdateDamageText()
{
	if (_damageTextTime > 0.0f) {
		_damageTextTime -= TIME->DeltaTime();
	}
	else {
		_damageTextTime = 0.0f;
		_cumulativeDamage = 0;
		if (_enemyStateUI->IsRenderActive())
			_enemyStateUI->SetRenderActive(false);
	}
}

void BossScript::AnimationEventListener(AnimationEvent event)
{
	switch (event.type) {
		case AnimationEventTypes::Attack: {
			bool attackFlag = event.datas[2].x == 1;

			_axeRb->SetPhysicsActive(attackFlag);

			if (attackFlag) {
				//AttackInfo attackInfo;
				_attackInfo.attakee = _gameObject;
				_attackInfo.damage = event.datas[0].w;
				_axeRb->customData = &_attackInfo;
			}

			break;
		}
		case AnimationEventTypes::Sound: {
			if (event.strData == "EnemyAttack")
				_axeAs->Play();
			break;
		}
		case AnimationEventTypes::Step: {
			_footAs->SetSound(_footstepSounds[Utils::Random(0, 4)]);
			_footAs->Play();
			break;
		}
		case AnimationEventTypes::RotateToTarget: {
			_rotateToTarget = event.datas[0].x == 1;
			break;
		}
	}
}

void BossScript::IdleState::StateStart(BossScript* owner)
{
	owner->_animator->SetCurrentAnimation("idle", 0.0f);
	owner->_animator->SetLoop(true);
}

void BossScript::IdleState::StateUpdate(BossScript* owner)
{
	int pattern = Utils::Random(0, 9);

	if (pattern <= 3) {
		owner->SetState(BossState::WALK);
	}
	else {
		owner->SetState(BossState::ATTACK);
	}
}

void BossScript::TrackWalkState::StateStart(BossScript* owner)
{
	patternTime = Utils::Random(2, 3);
	int direction = Utils::Random(0, 2);

	switch (direction) {
		case 0: {
			movingDir = MovingDirection::FRONT;
			owner->_animator->SetCurrentAnimation("walk_forward");
			break;
		}
		case 1: {
			movingDir = MovingDirection::LEFT;
			owner->_animator->SetCurrentAnimation("walk_left");
			break;
		}
		case 2: {
			movingDir = MovingDirection::RIGHT;
			owner->_animator->SetCurrentAnimation("walk_right");
			break;
		}
	}

	owner->_animator->SetLoop(true);
}

void BossScript::TrackWalkState::StateUpdate(BossScript* owner)
{
	if (owner->_targetDistance <= 2.0f) {
		owner->SetState(BossState::ATTACK);
		return;
	}

	if (patternTime > 0.0f) {
		owner->_transform->LookAtWithNoRoll(
			owner->_transform->GetPosition() -
			MathHelper::InterpolateVector(
				owner->_transform->GetBack(),
				owner->_targetVec,
				18.0f
			)
		);

		if (movingDir == MovingDirection::FRONT)
			owner->_controller->SetVelocity(-owner->_transform->GetLook() * 1.1f);
		else if (movingDir == MovingDirection::LEFT)
			owner->_controller->SetVelocity(-owner->_transform->GetLeft() * 1.1f);
		else if (movingDir == MovingDirection::RIGHT)
			owner->_controller->SetVelocity(-owner->_transform->GetRight() * 1.1f);

		patternTime -= TIME->DeltaTime();
	}
	else {
		int rand = Utils::Random(0, 6);
		if (rand <= 2) {
			patternTime = Utils::Random(2, 3);
			int direction = Utils::Random(0, 2);

			switch (direction) {
			case 0: {
				movingDir = MovingDirection::FRONT;
				owner->_animator->SetCurrentAnimation("walk_forward");
				break;
			}
			case 1: {
				movingDir = MovingDirection::LEFT;
				owner->_animator->SetCurrentAnimation("walk_left");
				break;
			}
			case 2: {
				movingDir = MovingDirection::RIGHT;
				owner->_animator->SetCurrentAnimation("walk_right");
				break;
			}
			}
		}
		else {
			owner->SetState(BossState::ATTACK);
		}
	}
}

void BossScript::DeathState::StateStart(BossScript* owner)
{
	owner->_animator->SetCurrentAnimation("death1");
	owner->_animator->SetLoop(false);
}

void BossScript::AttackState::StateStart(BossScript* owner)
{
	_patternIdx = Utils::Random(0, 3);
	_isAttackStarted = false;

	if (owner->_targetDistance > 2.0f) {
		owner->_animator->SetCurrentAnimation("run_forward");
		owner->_animator->SetLoop(true);
	}
}

void BossScript::AttackState::StateUpdate(BossScript* owner)
{
	if (_isAttackStarted) {
		if (owner->_animator->IsCurrentAnimationEnd()) {
			_isAttackStarted = false;
			owner->SetState(BossState::IDLE);
		}
		else if (owner->_rotateToTarget) {
			owner->_transform->LookAtWithNoRoll(
				owner->_transform->GetPosition() - 
				MathHelper::InterpolateVector(
					owner->_transform->GetBack(),
					owner->_targetVec,
					18.0f
				)
			);
		}
	}

	if (!_isAttackStarted) {
		if (owner->_targetDistance > 2.0f) {
			owner->_transform->LookAtWithNoRoll(
				owner->_transform->GetPosition() -
				MathHelper::InterpolateVector(
					owner->_transform->GetBack(),
					owner->_targetVec,
					18.0f
				)
			);
			owner->_controller->SetVelocity(-owner->_transform->GetLook() * 2.5f);
		}
		else {
			_isAttackStarted = true;
			switch (_patternIdx) {
				case 0: {
					owner->_animator->SetCurrentAnimation("axe_attack_downup_1");
					break;
				}
				case 1: {
					owner->_animator->SetCurrentAnimation("axe_attack_360_1");
					break;
				}
				case 2: {
					owner->_animator->SetCurrentAnimation("axe_attack_360_2");
					break;
				}
				case 3: {
					owner->_animator->SetCurrentAnimation("axe_attack_combo_2");
					break;
				}
			}

			owner->_animator->SetLoop(false);
		}
	}
}
