#pragma once
#include "Script.h"
#include "BaseState.h"

enum class EnemyState
{
    IDLE,
    WALK,
	ATTACK,
    DEATH
};

class EnemyScript : public Script
{
	enum class MovingDirection {
		FRONT,
		LEFT,
		RIGHT
	};

	class IdleState : public BaseState<EnemyScript> {
	public:
		void StateStart(EnemyScript* owner) override;
		void StateUpdate(EnemyScript* owner) override;
	};

	class TrackWalkState : public BaseState<EnemyScript> {
	public:
		void StateStart(EnemyScript* owner) override;
		void StateUpdate(EnemyScript* owner) override;
	private:
		float patternTime;
		MovingDirection movingDir;
	};

	class AttackState : public BaseState<EnemyScript> {
	public:
		void StateStart(EnemyScript* owner) override;
		void StateUpdate(EnemyScript* owner) override;
	private:
		int _patternIdx;
		bool _isAttackStarted;
	};

	class DeathState : public BaseState<EnemyScript> {
	public:
		void StateStart(EnemyScript* owner) override;
	};

public:
	~EnemyScript();

    void Init() override;
	void Update() override;

	void OnCollisionEnter(shared_ptr<GameObject> other) override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;
	void SaveXML(Bulb::XMLElement compElem) override;

public:
    void TakeDamage(int damage);
	shared_ptr<Transform> GetCenterTransform() { return _centerTransform; }

private:
	void SetState(EnemyState state) {
		if (_currentState == state) return;
		_currentState = state;
		_isStateChanged = true;
	}

	void SetDamageText(int damage);
	void UpdateDamageText();

public:
	shared_ptr<GameObject> target;

	bool blockExecute = true;

private:
	shared_ptr<GameObject> _gameObject;
	shared_ptr<Transform> _transform;
	shared_ptr<Transform> _centerTransform;
	shared_ptr<Animator> _animator;
	shared_ptr<CharacterController> _controller;
	shared_ptr<Rigidbody> _hitbox;
	shared_ptr<Rigidbody> _axeRb;

    int _health = 100;
    EnemyState _currentState = EnemyState::IDLE;
    bool _isStateChanged = false;

    vector<BaseState<EnemyScript>*> _patterns;
	Bulb::Vector3 _targetVec;

	shared_ptr<UIElement> _enemyStateUI;
	shared_ptr<UIText> _damageText;
	shared_ptr<UISlider> _healthBarUI;
	float _damageTextTime = 0.0f;
	int _cumulativeDamage = 0;

    float _targetDistance;
};
