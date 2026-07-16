#pragma once
#include "Component.h"

#define GRAVITY -9.81f

enum class BULB_API ColliderShape
{
	Box,
	Sphere,
	Capsule
};

class BULB_API Rigidbody : public Component
{
	friend class EditorGUIManager;
	using Super = Component;
public:
	Rigidbody();
	virtual ~Rigidbody();

	void Init() override;
	void PreUpdate() override;
	void Update() override;

#ifdef BULB_EDITOR
	virtual bool ShowComponentEditorGUI() override;
#endif

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;
	void SaveXML(Bulb::XMLElement compElem) override;

	shared_ptr<Component> Duplicate() override;

	ComponentSnapshot CaptureSnapshot() override;
	void RestoreSnapshot(ComponentSnapshot snapshot) override;

public:
	// Box Only
	Bulb::Vector3 GetColliderExtents() { return _extents; }
	void SetColliderExtents(const Bulb::Vector3& size);

	void SetColliderTrigger(bool value);

	// Capsule Only
	void SetColliderHalfHeight(float height);

	// Sphere, Capsule Only
	void SetColliderRadius(float radius);

	void SetColliderOffset(const Bulb::Vector3& offset);

	void SetColliderRotationOffset(const Bulb::Vector3& angle);

	Bulb::Vector3 GetColliderOffset() { return _colliderOffset; }

	ColliderShape GetColliderShape() { return _colliderShape; }
	void SetColliderShape(ColliderShape colliderShape);

	bool IsGravity() { return _isGravity; }
	void SetGravity(bool value);

	bool IsStatic() { return _isStatic; }
	void SetStatic(bool value);

	Bulb::Vector3 GetVelocity();
	void SetVelocity(Bulb::Vector3& velocity);

	bool IsPhysicsActive() { return _isPhysicsActive; }
	void SetPhysicsActive(bool value);

	BodyID GetBodyID() { return _bodyID; }

private:
	void CreateShape();
	void UpdateShapeData();
	JPH::ShapeSettings::ShapeResult FitOnMesh();

public:
	float customData;

private:
	bool _isGravity = true;				// 중력 여부
	bool _isStatic = false;
	bool _isTrigger = false;
	bool _isPhysicsActive = true;

	// Collider
	ColliderShape _colliderShape = ColliderShape::Box;
	Bulb::Vector3 _extents = { 0.5f, 0.5f, 0.5f };
	float _height = 0.5f;
	float _radius = 0.5f;
	Bulb::Vector3 _colliderOffset = { 0.0f, 0.0f, 0.0f };
	Bulb::Vector3 _colliderRotationOffset = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4 _rotationOffsetQuaternion;

	bool _shapeDataDirtyFlag = false;
	bool _colliderOffsetDirtyFlag = false;

	JPH::BodyID _bodyID;
	JPH::ShapeSettings::ShapeResult _shapeResult;
};

