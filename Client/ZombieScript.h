#pragma once
#include "EntityBase.h"
#include "BaseState.h"
#include "CommonStructs.h"

enum class ZombieState {
	Init,
	Scream,
	Walk,
	Attack,
	ReactionHit,
	Death
};

class ZombieScript : public EntityBase
{
#pragma region Player State Class
	class InitState : public BaseState<ZombieScript> {
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
	void TakeDamage(int damage) override;

	int GetCurrentHealth() { return _health; }

	void HitDelay() override;

private:
	void SetState(ZombieState state, bool forced = false) {
		if (_currentState == state && !forced) return;
		_currentState = state;
		_states[static_cast<int>(_currentState)]->StateStart(this);
	}

	void SetDamageText(int damage);
	void UpdateDamageText();

	void AnimationEventListener(AnimationEvent event) override;

private:
	shared_ptr<Rigidbody> _hitbox;

	ZombieState _currentState = ZombieState::Init;

	vector<BaseState<ZombieScript>*> _states;
	Bulb::Vector3 _targetVec;
	float _targetDistance;
	bool _isTargetLockedOn = false;

	shared_ptr<UIElement> _enemyStateUI;
	shared_ptr<UIText> _damageText;
	shared_ptr<UISlider> _healthBarUI;
	float _damageTextTime = 0.0f;
	int _cumulativeDamage = 0;

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

