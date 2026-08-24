#pragma once
#include "Script.h"
#include "BaseState.h"
#include "CommonStructs.h"

enum class BossState
{
    IDLE,
    WALK,
	ATTACK,
    DEATH
};

class BossScript : public Script
{
	enum class MovingDirection {
		FRONT,
		LEFT,
		RIGHT
	};

	class IdleState : public BaseState<BossScript> {
	public:
		void StateStart(BossScript* owner) override;
		void StateUpdate(BossScript* owner) override;
	};

	class TrackWalkState : public BaseState<BossScript> {
	public:
		void StateStart(BossScript* owner) override;
		void StateUpdate(BossScript* owner) override;
	private:
		float patternTime;
		MovingDirection movingDir;
	};

	class AttackState : public BaseState<BossScript> {
	public:
		void StateStart(BossScript* owner) override;
		void StateUpdate(BossScript* owner) override;
	private:
		int _patternIdx;
		bool _isAttackStarted;
	};

	class DeathState : public BaseState<BossScript> {
	public:
		void StateStart(BossScript* owner) override;
	};

public:
	~BossScript();

    void Init() override;
	void Update() override;

	void OnCollisionEnter(shared_ptr<GameObject> other) override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;
	void SaveXML(Bulb::XMLElement compElem) override;

public:
    void TakeDamage(int damage);
	shared_ptr<Transform> GetCenterTransform() { return _centerTransform; }

	void HitDelay();

private:
	void SetState(BossState state) {
		if (_currentState == state) return;
		_currentState = state;
		_isStateChanged = true;
	}

	void SetDamageText(int damage);
	void UpdateDamageText();

	void AnimationEventListener(AnimationEvent event);

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
    BossState _currentState = BossState::IDLE;
    bool _isStateChanged = false;

    vector<BaseState<BossScript>*> _patterns;
	Bulb::Vector3 _targetVec;

	shared_ptr<UIElement> _enemyStateUI;
	shared_ptr<UIText> _damageText;
	shared_ptr<UISlider> _healthBarUI;
	float _damageTextTime = 0.0f;
	int _cumulativeDamage = 0;

	AttackInfo _attackInfo;

	bool _isOnHitDelay = false;
	float _hitDelayTime = 0.0f;

    float _targetDistance;

	bool _rotateToTarget = false;

	AudioClip _footstepSounds[5];
	AudioClip _hitSound;
	AudioClip _axeSound;

	shared_ptr<AudioSource> _footAs;
	shared_ptr<AudioSource> _bodyAs;
	shared_ptr<AudioSource> _axeAs;
};
