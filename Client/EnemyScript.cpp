#include "pch.h"
#include "EnemyScript.h"

using namespace Bulb;

REGISTER_COMPONENT(EnemyScript)

EnemyScript::~EnemyScript()
{
	for (auto pattern : _patterns) {
		delete pattern;
	}
}

void EnemyScript::Init()
{
	_gameObject = GetGameObject();
	_gameObject->SetTag("Enemy");

	_transform = _gameObject->GetTransform();
	_centerTransform = _transform->GetChild("mixamorig:Hips");	// Mixamo Default Rig Name

	_animator = _gameObject->GetComponent<Animator>();
	_animator->SetLoop(true);

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
}

void EnemyScript::Update()
{
	if (blockExecute)
		return;

	UpdateDamageText();

	_targetVec = target->GetTransform()->GetPosition() - _transform->GetPosition();
	_targetVec.y = 0;

	if (_currentState != EnemyState::DEATH) {
		if (_health <= 0) {
			SetState(EnemyState::DEATH);
		}

		else if (_currentState == EnemyState::IDLE) {
			int pattern = Utils::Random(0, 9);

			if (/*pattern <= 3*/true) {
				SetState(EnemyState::WALK);
			}
			else {
				SetState(EnemyState::ATTACK);
			}

			//_targetDistance = _targetVec.Length();
			//if (_targetDistance >= 2.0f && _currentState != EnemyMovementState::WALK) {
			//	SetState(EnemyMovementState::WALK);
			//}
			//else if (_targetDistance < 2.0f && _currentState != EnemyMovementState::IDLE) {
			//	SetState(EnemyMovementState::IDLE);
			//}
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

void EnemyScript::OnCollisionEnter(shared_ptr<GameObject> other)
{
	if (other->GetTag() == "AttackAlly") {
		if (_health > 0) {
			TakeDamage(other->GetComponent<Rigidbody>()->customData);
		}
	}
}

void EnemyScript::OnDestroy()
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

void EnemyScript::LoadXML(Bulb::XMLElement compElem)
{

}

void EnemyScript::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "EnemyScript");
}

void EnemyScript::TakeDamage(int damage)
{
	_health -= damage;
	DEBUG->Log("Take" + to_string(damage) + "damage. Remain Health - " + to_string(_health));
	SetDamageText(damage);
	_healthBarUI->SetValue(_health);
}

void EnemyScript::SetDamageText(int damage)
{
	_damageTextTime = 2.0f;
	_cumulativeDamage += damage;
	_damageText->SetText(to_wstring(_cumulativeDamage));

	if (!_enemyStateUI->IsRenderActive())
		_enemyStateUI->SetRenderActive(true);
}

void EnemyScript::UpdateDamageText()
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

void EnemyScript::IdleState::StateStart(EnemyScript* owner)
{
	owner->_animator->SetCurrentAnimation("idle");
}

void EnemyScript::IdleState::StateUpdate(EnemyScript* owner)
{
	owner->_transform->LookAtWithNoRoll(-owner->_targetVec + owner->_transform->GetPosition());
}

void EnemyScript::TrackWalkState::StateStart(EnemyScript* owner)
{
	patternTime = Utils::Random(1, 3);
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

void EnemyScript::TrackWalkState::StateUpdate(EnemyScript* owner)
{
	if (patternTime > 0.0f) {
		owner->_transform->LookAtWithNoRoll(-owner->_targetVec + owner->_transform->GetPosition());

		if (movingDir == MovingDirection::FRONT)
			owner->_controller->SetVelocity(-owner->_transform->GetLook() * 1.1f);
		else if (movingDir == MovingDirection::LEFT)
			owner->_controller->SetVelocity(-owner->_transform->GetLeft() * 1.1f);
		else if (movingDir == MovingDirection::RIGHT)
			owner->_controller->SetVelocity(-owner->_transform->GetRight() * 1.1f);

		patternTime -= TIME->DeltaTime();
	}
	else {
		owner->SetState(EnemyState::IDLE);
	}
}

void EnemyScript::DeathState::StateStart(EnemyScript* owner)
{
	owner->_animator->SetCurrentAnimation("death1");
	owner->_animator->SetLoop(false);
}

void EnemyScript::AttackState::StateStart(EnemyScript* owner)
{

}

void EnemyScript::AttackState::StateUpdate(EnemyScript* owner)
{

}
