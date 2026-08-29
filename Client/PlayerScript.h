#pragma once
#include "Script.h"
#include "BaseState.h"
#include "CommonStructs.h"

class TPVCamera;
class Interactable;

enum class PlayerMovementState
{
	IDLE,
	WALK,
	RUN,
	SLASH,
	ROLL,
	STRAFE_FORWARD,
	STRAFE_BACK,
	STRAFE_RIGHT,
	STRAFE_LEFT,
	INTERACT,
	ENTER_VEIL
};

class PlayerScript : public Script
{
#pragma region Player State Class
	class IdleState : public BaseState<PlayerScript> {
	public:
		void StateStart(PlayerScript* owner) override;
		void StateUpdate(PlayerScript* owner) override;
	};

	class WalkState : public BaseState<PlayerScript> {
	public:
		void StateStart(PlayerScript* owner) override;
		void StateUpdate(PlayerScript* owner) override;
	};

	class RunState : public BaseState<PlayerScript> {
	public:
		void StateStart(PlayerScript* owner) override;
		void StateUpdate(PlayerScript* owner) override;
	};

	class SlashState : public BaseState<PlayerScript> {
	public:
		void StateStart(PlayerScript* owner) override;
		void StateUpdate(PlayerScript* owner) override;
	};

	class RollState : public BaseState<PlayerScript> {
	public:
		void StateStart(PlayerScript* owner) override;
		void StateUpdate(PlayerScript* owner) override;
	};

	class StrafeForwardState : public BaseState<PlayerScript> {
	public:
		void StateStart(PlayerScript* owner) override;
		void StateUpdate(PlayerScript* owner) override;
	};

	class StrafeBackState : public BaseState<PlayerScript> {
	public:
		void StateStart(PlayerScript* owner) override;
		void StateUpdate(PlayerScript* owner) override;
	};

	class StrafeRightState : public BaseState<PlayerScript> {
	public:
		void StateStart(PlayerScript* owner) override;
		void StateUpdate(PlayerScript* owner) override;
	};

	class StrafeLeftState : public BaseState<PlayerScript> {
	public:
		void StateStart(PlayerScript* owner) override;
		void StateUpdate(PlayerScript* owner) override;
	};

	class InteractState : public BaseState<PlayerScript> {
	public:
		void StateStart(PlayerScript* owner) override;
		void StateUpdate(PlayerScript* owner) override;
	};

	class EnterVeilState : public BaseState<PlayerScript> {
	public:
		void StateStart(PlayerScript* owner) override;
		void StateUpdate(PlayerScript* owner) override;
	};
#pragma endregion

public:
	~PlayerScript();

	void Init() override;
	void Update() override;

	void OnCollisionEnter(shared_ptr<GameObject> other) override;

	void OnCollisionExit(shared_ptr<GameObject> other) override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;
	void SaveXML(Bulb::XMLElement compElem) override;

public:
	void Roll();
	void Attack();
	void Move();
	void LockOn();

	void SetState(PlayerMovementState state) {
		if (_playerMovementState == state) return;
		_playerMovementState = state;
		_isStateChanged = true;
	}

	void HitDelay();

private:
	void Interact();

	void RecoveryStemina();

	void TakeDamage(int damage);

	// Decrease stemina per second
	void DecreaseStemina(float value, bool instantChange = true);

	void AnimationEventListener(AnimationEvent event);

public:
	shared_ptr<TPVCamera> tpvCameraScript;

	float health = 100.0f;
	float steminaMax = 6000.0f;
	float stemina = 6000.0f;

private:
	shared_ptr<GameObject> _gameObject;
	shared_ptr<Transform> _transform;
	shared_ptr<Animator> _animator;
	shared_ptr<CharacterController> _controller;
	shared_ptr<UISlider> _hpBar;
	shared_ptr<UISlider> _steminaBar;
	shared_ptr<Rigidbody> _swordRb;
	shared_ptr<AudioSource> _swordAs;

	float _speed = 1.55f;
	float _rotationSpeed = 17.0f;

	PlayerMovementState _playerMovementState = PlayerMovementState::IDLE;
	bool _isStateChanged = false;

	shared_ptr<GameObject> _lockOnTarget;
	bool _isLockOn = false;

	Bulb::Vector3 _movingDirection;

	vector<BaseState<PlayerScript>*> _states;

	float _recoverySteminaDelayedTime = 0.0f;
	bool _isRecoveryPossible = true;

	bool _isEvading = false;

	AttackInfo _attackInfo;

	bool _isOnHitDelay = false;
	float _hitDelayTime = 0.0f;

	vector<shared_ptr<Interactable>> _interactableScripts;

	AudioClip _playerFootstepSounds[5];
	AudioClip _hitSound;

	shared_ptr<AudioSource> _playerFootAs;
	shared_ptr<AudioSource> _hitAs;
};
