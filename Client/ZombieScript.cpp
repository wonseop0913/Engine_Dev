#include "pch.h"
#include "ZombieScript.h"
#include "PlayerScript.h"

using namespace Bulb;

REGISTER_COMPONENT(ZombieScript)

void ZombieScript::Init()
{
	_gameObject = GetGameObject();
	_transform = GetTransform();
	_centerTransform = _transform->GetChild("mixamorig5:Spine1");
	_animator = _gameObject->GetComponent<Animator>();
	_controller = _gameObject->GetComponent<CharacterController>();
	_hitbox = _gameObject->GetComponent<Rigidbody>();

	_animator->LoadAnimationEvents("..\\Resources\\Animations\\zombie\\AnimationEvents.xml");
	_animator->animationEvent += [this](AnimationEvent e) {
		this->AnimationEventListener(e);
	};

	_controller->SetHalfHeight(0.6f);
	_controller->SetRadius(0.3f);
	_controller->SetOffset(Vector3(0.0f, 0.9f, 0.0f));

	_patterns.push_back(new IdleState());
	_patterns.push_back(new ScreamState());
	_patterns.push_back(new WalkState());
	_patterns.push_back(new AttackState());
	_patterns.push_back(new ReactionHitState());
	_patterns.push_back(new DeathState());

	_isStateChanged = true;

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
	_healthBarUI->SetValueMaxLimit(_health);
	_healthBarUI->SetValueMinLimit(0.0f);
	_healthBarUI->SetValue(_health);

	_enemyStateUI->SetRenderActive(false);

	target = RENDER->GetObjectWithTag("Player");

	_headAs = _transform->GetChild("mixamorig5:Head")->GetGameObject()->GetComponent<AudioSource>();
	_bodyAs = _centerTransform->GetGameObject()->GetComponent<AudioSource>();
	_footAs = _transform->GetGameObject()->GetComponent<AudioSource>();

	_footstepSounds[0] = SOUND->LoadSound("Sounds/DirtWalk1.wav", false);
	_footstepSounds[1] = SOUND->LoadSound("Sounds/DirtWalk2.wav", false);
	_footstepSounds[2] = SOUND->LoadSound("Sounds/DirtWalk3.wav", false);
	_footstepSounds[3] = SOUND->LoadSound("Sounds/DirtWalk4.wav", false);
	_footstepSounds[4] = SOUND->LoadSound("Sounds/DirtWalk5.wav", false);

	_screamSound = SOUND->LoadSound("Sounds/ZombieScream.wav", false);
	_attackSound = SOUND->LoadSound("Sounds/Axe.wav", false);
	_attackNoiseSound = SOUND->LoadSound("Sounds/ZombieAttackNoise.wav", false);
	_reactionHitSound = SOUND->LoadSound("Sounds/ZombieReactionHit.wav", false);
	_hitSound = SOUND->LoadSound("Sounds/SwordHit1.wav", false);
	_deathSound = SOUND->LoadSound("Sounds/ZombieDeath.wav", false);
	_deathEndSound = SOUND->LoadSound("Sounds/ZombieDeathEnd.wav", false);
}

void ZombieScript::Update()
{
	UpdateDamageText();

	_targetVec = target->GetTransform()->GetPosition() - _transform->GetPosition();
	_targetVec.y = 0;
	_targetDistance = _targetVec.Length();

	//if (_currentState != ZombieState::DEATH) {
	//	if (_health <= 0) {
	//		SetState(ZombieState::DEATH);
	//	}
	//}

	if (_isStateChanged) {
		_patterns[static_cast<int>(_currentState)]->StateStart(this);
		_isStateChanged = false;
	}

	_patterns[static_cast<int>(_currentState)]->StateUpdate(this);

	Vector3 pos = _transform->GetPosition();
	_enemyStateUI->GetTransform()->SetPosition({ pos.x, pos.y + 2.2f, pos.z });
}

void ZombieScript::OnDestroy()
{
	target.reset();

	_gameObject.reset();
	_transform.reset();
	_centerTransform.reset();
	_animator.reset();
	_controller.reset();

	for (auto& pattern : _patterns) {
		delete pattern;
	}

	_enemyStateUI.reset();
	_damageText.reset();
	_healthBarUI.reset();
}

void ZombieScript::OnCollisionEnter(shared_ptr<GameObject> other)
{
	if (other->GetTag() == "AttackAlly") {
		if (_health > 0) {
			AttackInfo* attackInfo = (AttackInfo*)(other->GetComponent<Rigidbody>()->customData);
			TakeDamage(attackInfo->damage);
			target->GetComponent<PlayerScript>()->HitDelay();

			_headAs->SetSound(_reactionHitSound);
			_headAs->Play();
			_bodyAs->SetSound(_hitSound);
			_bodyAs->Play();
			if (_health > 0) {
				_isStateChanged = true;
				SetState(ZombieState::ReactionHit);
			}
			else {
				SetState(ZombieState::Death);
			}
		}
	}
}

void ZombieScript::LoadXML(Bulb::XMLElement compElem)
{

}

void ZombieScript::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "ZombieScript");
}

void ZombieScript::TakeDamage(int damage)
{
	_health -= damage;
	DEBUG->Log("Take" + to_string(damage) + "damage. Remain Health - " + to_string(_health));
	SetDamageText(damage);
	_healthBarUI->SetValue(_health);
	_bodyAs->Play();
}

void ZombieScript::SetDamageText(int damage)
{
	_damageTextTime = 2.0f;
	_cumulativeDamage += damage;
	_damageText->SetText(to_wstring(_cumulativeDamage));

	if (!_enemyStateUI->IsRenderActive())
		_enemyStateUI->SetRenderActive(true);
}

void ZombieScript::UpdateDamageText()
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

void ZombieScript::AnimationEventListener(AnimationEvent event)
{
	switch (event.type) {
	case AnimationEventTypes::Sound: {
		if (event.strData == "ZombieScream") {
			_headAs->SetSound(_screamSound);
			_headAs->Play();
		}
		if (event.strData == "ZombieAttack") {
			_bodyAs->SetSound(_attackSound);
			_bodyAs->Play();
		}
		if (event.strData == "ZombieAttack") {
			_headAs->SetSound(_attackNoiseSound);
			_headAs->Play();
		}
		if (event.strData == "ZombieDeath") {
			_headAs->SetSound(_deathSound);
			_headAs->Play();
		}
		if (event.strData == "ZombieDeathEnd") {
			_headAs->SetSound(_deathEndSound);
			_headAs->Play();
		}
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

void ZombieScript::IdleState::StateStart(ZombieScript* owner)
{
	owner->_animator->SetCurrentAnimation("idle");
	owner->_animator->SetLoop(false);
	owner->_animator->PauseAnimation();
}

void ZombieScript::IdleState::StateUpdate(ZombieScript* owner)
{
	if (owner->_currentState == ZombieState::Idle && owner->_targetDistance <= 5.0f) {
		owner->_animator->PlayAnimation();
		owner->_isTargetLockedOn = true;
	}
	if (!owner->_animator->IsInTransition() && owner->_animator->IsCurrentAnimationEnd())
		owner->SetState(ZombieState::Scream);
}

void ZombieScript::ScreamState::StateStart(ZombieScript* owner)
{
	owner->_animator->SetCurrentAnimation("scream");
	owner->_animator->SetLoop(false);
}

void ZombieScript::ScreamState::StateUpdate(ZombieScript* owner)
{
	if (!owner->_animator->IsInTransition() && owner->_animator->IsCurrentAnimationEnd()) {
		owner->SetState(ZombieState::Walk);
	}
}

void ZombieScript::WalkState::StateStart(ZombieScript* owner)
{
	owner->_animator->SetCurrentAnimation("walk_forward");
	owner->_animator->SetLoop(true);
}

void ZombieScript::WalkState::StateUpdate(ZombieScript* owner)
{
	if (owner->_targetDistance <= 2.0f) {
		owner->SetState(ZombieState::Attack);
		return;
	}

	owner->_transform->LookAtWithNoRoll(
		owner->_transform->GetPosition() -
		MathHelper::InterpolateVector(
			owner->_transform->GetBack(),
			owner->_targetVec,
			8.0f
		)
	);

	owner->_controller->SetVelocity(-owner->_transform->GetLook() * 0.4f);
}

void ZombieScript::AttackState::StateStart(ZombieScript* owner)
{
	owner->_animator->SetCurrentAnimation("attack1");
	owner->_animator->SetLoop(false);
}

void ZombieScript::AttackState::StateUpdate(ZombieScript* owner)
{
	if (!owner->_animator->IsInTransition() && owner->_animator->IsCurrentAnimationEnd()) {
		owner->SetState(ZombieState::Walk);
	}

	if (owner->_rotateToTarget) {
		owner->_transform->LookAtWithNoRoll(
			owner->_transform->GetPosition() -
			MathHelper::InterpolateVector(
				owner->_transform->GetBack(),
				owner->_targetVec,
				5.0f
			)
		);
	}
}

void ZombieScript::ReactionHitState::StateStart(ZombieScript* owner)
{
	owner->_animator->SetCurrentAnimation("reaction_hit", 0.0f);
	owner->_animator->SetLoop(false);
}

void ZombieScript::ReactionHitState::StateUpdate(ZombieScript* owner)
{
	if (owner->_animator->IsCurrentAnimationEnd()) {
		owner->SetState(ZombieState::Walk);
	}
}

void ZombieScript::DeathState::StateStart(ZombieScript* owner)
{
	owner->_animator->SetCurrentAnimation("death");
	owner->_animator->SetLoop(false);
	owner->_gameObject->SetTag("EnemyDeath");
	owner->_hitbox->SetPhysicsActive(false);
}
