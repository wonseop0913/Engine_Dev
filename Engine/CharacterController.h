#pragma once
#include "Component.h"

class BULB_API CharacterController : public Component
{
public:
	CharacterController();
	~CharacterController() override;

	void Init() override;
	void PreUpdate() override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;
	void SaveXML(Bulb::XMLElement compElem) override;

	shared_ptr<Component> Duplicate() override;

	ComponentSnapshot CaptureSnapshot() override;
	void RestoreSnapshot(ComponentSnapshot snapshot) override;

#ifdef BULB_EDITOR
	bool ShowComponentEditorGUI() override;
#endif

public:
	void SetHalfHeight(float height) { _height = height; }
	void SetRadius(float radius) { _radius = radius; }
	void SetOffset(Bulb::Vector3 offset) { _offset = offset; }

	void SetVelocity(Bulb::Vector3 velocity) { _desiredVelocity = velocity; }

	bool IsPhysicsActive() { return _isPhysicsActive; }
	void SetPhysicsActive(bool value);

	void SetGravity(bool value) { _isGravity = value; }

	bool IsOnGround();

private:
	JPH::Ref<JPH::CharacterVirtual> _character;
	JPH::Ref<JPH::Shape> _shape;
	JPH::BodyID _bodyId;

	float _height = 0.5f;
	float _radius = 0.5f;
	Bulb::Vector3 _offset = { 0.0f, 0.0f, 0.0f };

	Bulb::Vector3 _currentVelocity = { 0.0f, 0.0f, 0.0f };
	Bulb::Vector3 _desiredVelocity = { 0.0f, 0.0f, 0.0f };
	float _verticalVelocity = 0.0f;
	bool _isGravity = true;
	float _gravityFactor = 1.0f;

	bool _isPhysicsActive = true;
};

