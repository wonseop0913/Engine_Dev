#pragma once
#include "Script.h"
#include "BaseState.h"
#include "CommonStructs.h"

enum class ZombieState {
	Idle,
	Scream,
	Walk,
	Attack,
	ReactionHit,
	Death
};

class ZombieScript : public Script
{
#pragma region Player State Class
	class IdleState : public BaseState<ZombieScript> {
	public:
		void StateStart(ZombieScript* owner) override;
		void StateUpdate(ZombieScript* owner) override;
	};

	class ScreamState : public BaseState<ZombieScript> {
	public:
		void StateStart(ZombieScript* owner) override;
		void StateUpdate(ZombieScript* owner) override;
	};

	class WalkState : public BaseState<ZombieScript> {
	public:
		void StateStart(ZombieScript* owner) override;
		void StateUpdate(ZombieScript* owner) override;
	};

	class AttackState : public BaseState<ZombieScript> {
	public:
		void StateStart(ZombieScript* owner) override;
		void StateUpdate(ZombieScript* owner) override;
	};

	class ReactionHitState : public BaseState<ZombieScript> {
	public:
		void StateStart(ZombieScript* owner) override;
		void StateUpdate(ZombieScript* owner) override;
	};

	class DeathState : public BaseState<ZombieScript> {
	public:
		void StateStart(ZombieScript* owner) override;
	};
#pragma endregion

public:
	void Init() override;

	void Update() override;

	void OnDestroy() override;

	void OnCollisionEnter(shared_ptr<GameObject> other) override;

	void LoadXML(Bulb::XMLElement compElem) override;

	void SaveXML(Bulb::XMLElement compElem) override;

public:
	void TakeDamage(int damage);
	shared_ptr<Transform> GetCenterTransform() { return _centerTransform; }

	int GetCurrentHealth() { return _health; }

private:
	void SetState(ZombieState state) {
		if (_currentState == state) return;
		_currentState = state;
		_isStateChanged = true;
	}

	void SetDamageText(int damage);
	void UpdateDamageText();

	void AnimationEventListener(AnimationEvent event);

public:
	shared_ptr<GameObject> target;

private:
	shared_ptr<GameObject> _gameObject;
	shared_ptr<Transform> _transform;
	shared_ptr<Transform> _centerTransform;
	shared_ptr<Animator> _animator;
	shared_ptr<CharacterController> _controller;
	shared_ptr<Rigidbody> _hitbox;

	int _health = 30;
	ZombieState _currentState = ZombieState::Idle;
	bool _isStateChanged = false;

	vector<BaseState<ZombieScript>*> _patterns;
	Bulb::Vector3 _targetVec;
	float _targetDistance;
	bool _isTargetLockedOn = false;

	shared_ptr<UIElement> _enemyStateUI;
	shared_ptr<UIText> _damageText;
	shared_ptr<UISlider> _healthBarUI;
	float _damageTextTime = 0.0f;
	int _cumulativeDamage = 0;

	AttackInfo _attackInfo;

	bool _rotateToTarget = false;

	shared_ptr<AudioSource> _headAs;
	shared_ptr<AudioSource> _bodyAs;
	shared_ptr<AudioSource> _footAs;

	AudioClip _footstepSounds[5];
	AudioClip _screamSound;
	AudioClip _attackSound;
	AudioClip _attackNoiseSound;
	AudioClip _reactionHitSound;
	AudioClip _hitSound;
	AudioClip _deathSound;
	AudioClip _deathEndSound;
};

