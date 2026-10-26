#pragma once
#include "Script.h"
#include "CommonStructs.h"

class EntityBase : public Script
{
public:
	void OnDestroy() override {}

	void LoadXML(Bulb::XMLElement compElem) override {}

	void SaveXML(Bulb::XMLElement compElem) override {}

	virtual void TakeDamage(int damage) {}
	shared_ptr<Transform> GetCenterTransform() { return _centerTransform; }
	virtual void HitDelay() {}

protected:
	virtual void AnimationEventListener(AnimationEvent event) {}

public:
	shared_ptr<GameObject> targetObj;
	shared_ptr<Transform> targetTransform;

protected:
	shared_ptr<GameObject> _gameObject;
	shared_ptr<Transform> _transform;
	shared_ptr<Transform> _centerTransform;
	shared_ptr<Transform> _headTransform;
	shared_ptr<Animator> _animator;
	shared_ptr<CharacterController> _controller;

	int _healthMax = 100;
	int _health = 100;

	AttackInfo _attackInfo;

	bool _isOnHitDelay = false;
	float _hitDelayTime = 0.0f;
};
