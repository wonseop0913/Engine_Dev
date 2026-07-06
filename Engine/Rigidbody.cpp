#include "pch.h"
#include "Rigidbody.h"

// REGISTER_COMPONENT(Rigidbody);

Rigidbody::Rigidbody() : Super(ComponentType::Rigidbody)
{
	_isGravity = true;

	XMStoreFloat4(&_rotationOffsetQuaternion, XMQuaternionIdentity());
}

Rigidbody::~Rigidbody()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - Rigidbody:" << _id << "\n";
#endif
}

void Rigidbody::Init()
{
	auto transform = GetTransform();
	auto pos = transform->GetPosition();
	auto rot = transform->GetQuaternion();

	if (!_shapeDataDirtyFlag) {
		_shapeResult = FitOnMesh();
	}
	else {
		CreateShape();
	}

	// Body 생성 설정
	JPH::BodyCreationSettings bodySettings(_shapeResult.Get(),
		JPH::RVec3(pos.x, pos.y, pos.z),
		JPH::Quat(rot.x, rot.y, rot.z, rot.w),
		_isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
		_isStatic ? Layers::NON_MOVING : Layers::MOVING);

	bodySettings.mIsSensor = _isTrigger;
	bodySettings.mUserData = reinterpret_cast<JPH::uint64>(_gameObject.lock().get());
	bodySettings.mGravityFactor = _isGravity ? 1.0f : 0.0f;
	bodySettings.mAllowDynamicOrKinematic = true;

	// 시스템 등록
	JPH::BodyInterface& bodyInterface = PHYSICS->GetPhysicsSystem()->GetBodyInterface();
	_bodyID = bodyInterface.CreateAndAddBody(bodySettings, JPH::EActivation::Activate);

	if (!_isPhysicsActive)
		bodyInterface.RemoveBody(_bodyID);

	if (_shapeDataDirtyFlag) {
		_shapeDataDirtyFlag = false;
		UpdateShapeData();
	}

	PHYSICS->AddRigidbody(static_pointer_cast<Rigidbody>(shared_from_this()));
}

void Rigidbody::PreUpdate()
{
	if (!_isPhysicsActive) return;

	if (_gameObject.lock()->GetFramesDirty() > 0) {
		auto transform = _gameObject.lock()->GetTransform();
		auto pos = transform->GetPosition();
		auto rot = transform->GetQuaternion();

		XMVECTOR finalQuat = XMQuaternionMultiply(XMLoadFloat4(&_rotationOffsetQuaternion), XMLoadFloat4(&rot));

		pos = pos + XMVector3Rotate(XMLoadFloat3(&_colliderOffset), finalQuat);

		PHYSICS->GetPhysicsSystem()->GetBodyInterface().SetPositionAndRotation(
			_bodyID,
			JPH::RVec3(pos.x, pos.y, pos.z),
			JPH::Quat(finalQuat),
			JPH::EActivation::Activate
		);
	}
}

void Rigidbody::Update()
{

}

bool Rigidbody::ShowComponentEditorGUI()
{
	if (ImGui::CollapsingHeader("Rigidbody", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (_isPhysicsActive)
			ImGui::TextColored({ 0, 1, 0, 1 }, "Physics Active");
		else
			ImGui::TextColored({ 1, 0, 0, 1 }, "Physics Inactive");
		if (ImGui::Checkbox("Static", &_isStatic)) {
			PHYSICS->GetPhysicsSystem()->GetBodyInterface().SetMotionType(
				_bodyID,
				_isStatic ? EMotionType::Static : EMotionType::Dynamic,
				EActivation::Activate);
		}
		if (ImGui::Checkbox("Gravity", &_isGravity)) {
			PHYSICS->GetPhysicsSystem()->GetBodyInterface().SetGravityFactor(_bodyID, _isGravity ? 1.0f : 0.0f);
		}
		if (ImGui::Checkbox("Trigger", &_isTrigger)) {
			PHYSICS->GetPhysicsSystem()->GetBodyInterface().SetIsSensor(_bodyID, _isTrigger);
		}

		switch (_colliderShape) {
		case ColliderShape::Box: {
			static float extentsValues[3];
			extentsValues[0] = _extents.x;
			extentsValues[1] = _extents.y;
			extentsValues[2] = _extents.z;
			ImGui::SeparatorText("Extents");
			if (ImGui::InputFloat3("##ColliderExtents", extentsValues)) {
				SetColliderExtents({ extentsValues[0], extentsValues[1], extentsValues[2] });
			}
			break;
		}
		}
	}

	return false;
}

void Rigidbody::OnDestroy()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "OnDestroy - Rigidbody:" << _id << "\n";
#endif

	JPH::BodyInterface& bodyInterface = PHYSICS->GetPhysicsSystem()->GetBodyInterface();
	bodyInterface.RemoveBody(_bodyID);
	bodyInterface.DestroyBody(_bodyID);
	PHYSICS->DeleteRigidbody(static_pointer_cast<Rigidbody>(shared_from_this()));
}

void Rigidbody::LoadXML(Bulb::XMLElement compElem)
{
	SetStatic(compElem.BoolAttribute("Static", false));
	SetGravity(compElem.BoolAttribute("Gravity", true));
	SetColliderTrigger(compElem.BoolAttribute("Trigger", false));
	SetPhysicsActive(compElem.BoolAttribute("PhysicsActive", true));

	Bulb::XMLElement colliderElem = compElem.FirstChildElement("Collider");
	if (!colliderElem.IsNullPtr()) {
		const char* colliderShape = colliderElem.Attribute("Shape");
		if (colliderShape != 0) {
			string shapeStr(colliderShape);
			if (shapeStr == "Box") SetColliderShape(ColliderShape::Box);
			else if (shapeStr == "Sphere") SetColliderShape(ColliderShape::Sphere);
			else if (shapeStr == "Capsule") SetColliderShape(ColliderShape::Capsule);
		}

		Bulb::XMLElement extentElem = colliderElem.FirstChildElement("Extent");
		if (!extentElem.IsNullPtr()) {
			SetColliderExtents({ extentElem.FloatAttribute("x", 0.5f), extentElem.FloatAttribute("y", 0.5f), extentElem.FloatAttribute("z", 0.5f) });
		}
		SetColliderHalfHeight(colliderElem.FloatAttribute("Height", 0.5f));
		SetColliderRadius(colliderElem.FloatAttribute("Radius", 0.5f));

		Bulb::XMLElement offsetElem = colliderElem.FirstChildElement("Offset");
		if (!offsetElem.IsNullPtr()) {
			SetColliderOffset({ offsetElem.FloatAttribute("x", 0.0f), offsetElem.FloatAttribute("y", 0.0f), offsetElem.FloatAttribute("z", 0.0f) });
		}

		Bulb::XMLElement rotOffsetElem = colliderElem.FirstChildElement("RotOffset");
		if (!rotOffsetElem.IsNullPtr()) {
			SetColliderRotationOffset({ rotOffsetElem.FloatAttribute("x", 0.0f), rotOffsetElem.FloatAttribute("y", 0.0f), rotOffsetElem.FloatAttribute("z", 0.0f) });
		}
	}
}

void Rigidbody::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "Rigidbody");

	compElem.SetAttribute("Static", _isStatic);
	compElem.SetAttribute("Gravity", _isGravity);
	compElem.SetAttribute("Trigger", _isTrigger);
	compElem.SetAttribute("PhysicsActive", _isPhysicsActive);

	Bulb::XMLElement colliderElem = compElem.InsertNewChildElement("Collider");
	colliderElem.SetAttribute("Shape", magic_enum::enum_name(_colliderShape).data());
	colliderElem.SetAttribute("Height", _height);
	colliderElem.SetAttribute("Radius", _radius);

	Bulb::XMLElement extentElem = colliderElem.InsertNewChildElement("Extent");
	extentElem.SetAttribute("x", _extents.x);
	extentElem.SetAttribute("y", _extents.y);
	extentElem.SetAttribute("z", _extents.z);

	Bulb::XMLElement offsetElem = colliderElem.InsertNewChildElement("Offset");
	offsetElem.SetAttribute("x", _colliderOffset.x);
	offsetElem.SetAttribute("y", _colliderOffset.y);
	offsetElem.SetAttribute("z", _colliderOffset.z);

	Bulb::XMLElement rotOffsetElem = colliderElem.InsertNewChildElement("RotOffset");
	rotOffsetElem.SetAttribute("x", _colliderRotationOffset.x);
	rotOffsetElem.SetAttribute("y", _colliderRotationOffset.y);
	rotOffsetElem.SetAttribute("z", _colliderRotationOffset.z);
}

shared_ptr<Component> Rigidbody::Duplicate()
{
	shared_ptr<Rigidbody> comp = static_pointer_cast<Rigidbody>(ComponentFactory::Create("Rigidbody"));

	comp->SetStatic(_isStatic);
	comp->SetGravity(_isGravity);
	comp->SetColliderTrigger(_isTrigger);
	comp->SetPhysicsActive(_isPhysicsActive);
	comp->SetColliderShape(_colliderShape);
	comp->SetColliderHalfHeight(_height);
	comp->SetColliderRadius(_radius);
	comp->SetColliderExtents(_extents);
	comp->SetColliderOffset(_colliderOffset);
	comp->SetColliderRotationOffset(_colliderRotationOffset);

	return comp;
}

ComponentSnapshot Rigidbody::CaptureSnapshot()
{
	ComponentSnapshot snapshot;

	snapshot.id = _id;
	snapshot.componentType = "Rigidbody";

	snapshot.datas.push_back(_isGravity ? 1 : 0);
	snapshot.datas.push_back(_isStatic ? 1 : 0);
	snapshot.datas.push_back(_isTrigger ? 1 : 0);
	snapshot.datas.push_back(_isPhysicsActive ? 1 : 0);

	snapshot.datas.push_back(_shapeDataDirtyFlag ? 1 : 0);
	snapshot.datas.push_back(_colliderOffsetDirtyFlag ? 1 : 0);

	return snapshot;
}

void Rigidbody::RestoreSnapshot(ComponentSnapshot snapshot)
{
	SetGravity(snapshot.datas[0] == 1);
	SetStatic(snapshot.datas[1] == 1);
	SetColliderTrigger(snapshot.datas[2] == 1);
	SetPhysicsActive(snapshot.datas[3] == 1);

	_shapeDataDirtyFlag = snapshot.datas[4] == 1;
	_colliderOffsetDirtyFlag = snapshot.datas[5] == 1;

	PHYSICS->GetPhysicsSystem()->GetBodyInterface().SetLinearVelocity(_bodyID, { 0.0f, 0.0f, 0.0f });
	PHYSICS->GetPhysicsSystem()->GetBodyInterface().SetAngularVelocity(_bodyID, { 0.0f, 0.0f, 0.0f });
}

void Rigidbody::SetColliderExtents(const Bulb::Vector3& extents)
{
	_extents = extents;

	if (_colliderShape == ColliderShape::Box) {
		CreateShape();
		UpdateShapeData();
	}
}

void Rigidbody::SetColliderTrigger(bool value)
{
	if (value == _isTrigger) return;
	_isTrigger = value;

	if (!isInitialized) return;

	PHYSICS->GetPhysicsSystem()->GetBodyInterface().SetIsSensor(_bodyID, _isTrigger);
}

void Rigidbody::SetColliderHalfHeight(float height)
{
	_height = height;

	if (_colliderShape == ColliderShape::Capsule)
		UpdateShapeData();
}

void Rigidbody::SetColliderRadius(float radius)
{
	_radius = radius;

	if (_colliderShape == ColliderShape::Sphere)
		UpdateShapeData();
}

void Rigidbody::SetColliderOffset(const Bulb::Vector3& offset)
{
	_colliderOffset = offset;
}

void Rigidbody::SetColliderRotationOffset(const Bulb::Vector3& angle)
{
	_colliderRotationOffset = angle;
	Bulb::Vector3 radian = MathHelper::DegreeToRadian(_colliderRotationOffset);
	XMStoreFloat4(
		&_rotationOffsetQuaternion, 
		XMQuaternionRotationRollPitchYaw(radian.x, radian.y, radian.z)
	);
}

void Rigidbody::SetColliderShape(ColliderShape colliderShape)
{
	if (_colliderShape == colliderShape) return;

	_colliderShape = colliderShape;

	CreateShape();

	UpdateShapeData();
}

void Rigidbody::SetGravity(bool value)
{
	if (value == _isGravity) return;
	_isGravity = value;

	if (!isInitialized) return;

	PHYSICS->GetPhysicsSystem()->GetBodyInterface().SetGravityFactor(_bodyID, _isGravity ? 1.0f : 0.0f);
}

void Rigidbody::SetStatic(bool value)
{
	if (value == _isStatic) return;
	_isStatic = value;

	if (!isInitialized) return;

	PHYSICS->GetPhysicsSystem()->GetBodyInterface().SetMotionType(
		_bodyID,
		_isStatic ? EMotionType::Static : EMotionType::Dynamic,
		EActivation::Activate);
}

Bulb::Vector3 Rigidbody::GetVelocity() {
	JPH::Vec3 vel = PHYSICS->GetPhysicsSystem()->GetBodyInterface().GetLinearVelocity(_bodyID);
	return Bulb::Vector3(vel.mValue);
}

void Rigidbody::SetVelocity(Bulb::Vector3& velocity) {
	PHYSICS->GetPhysicsSystem()->GetBodyInterface().SetLinearVelocity(_bodyID, JPH::Vec3(velocity.x, velocity.y, velocity.z));
}

void Rigidbody::SetPhysicsActive(bool value)
{
	if (value == _isPhysicsActive) return;
	_isPhysicsActive = value;

	if (!isInitialized) return;

	if (_isPhysicsActive)
		PHYSICS->GetPhysicsSystem()->GetBodyInterface().AddBody(_bodyID, EActivation::Activate);
	else
		PHYSICS->GetPhysicsSystem()->GetBodyInterface().RemoveBody(_bodyID);
}

void Rigidbody::CreateShape()
{
	if (!isInitialized) {
		_shapeDataDirtyFlag = true;
		return;
	}

	if (_colliderShape == ColliderShape::Box) {
		JPH::BoxShapeSettings boxShapeSetting(JPH::Vec3(_extents.x, _extents.y, _extents.z));
		_shapeResult = boxShapeSetting.Create();
	}
	else if (_colliderShape == ColliderShape::Sphere) {
		JPH::SphereShapeSettings sphereShapeSetting(_radius);
		_shapeResult = sphereShapeSetting.Create();
	}
	else if (_colliderShape == ColliderShape::Capsule) {
		JPH::CapsuleShapeSettings capsuleShapeSetting;
		capsuleShapeSetting.mHalfHeightOfCylinder = _height;
		capsuleShapeSetting.mRadius = _radius;
		_shapeResult = capsuleShapeSetting.Create();
	}
}

void Rigidbody::UpdateShapeData()
{
	if (!isInitialized) {
		_shapeDataDirtyFlag = true;
		return;
	}

	JPH::BodyInterface& bodyInterface = PHYSICS->GetPhysicsSystem()->GetBodyInterface();
	bodyInterface.SetShape(
		_bodyID, 
		_shapeResult.Get(), 
		true, 
		_isPhysicsActive ? JPH::EActivation::Activate : JPH::EActivation::DontActivate
	);
}

JPH::ShapeSettings::ShapeResult Rigidbody::FitOnMesh()
{
	auto meshRenderer = GetGameObject()->GetComponent<MeshRenderer>();
	if (meshRenderer == nullptr)
	{
		DEBUG->WarnLog("BoxCollider::FitOnMesh() - MeshRenderer not found.");
		JPH::BoxShapeSettings shapeSettings(JPH::Vec3(1.0f, 1.0f, 1.0f));
		return shapeSettings.Create();
	}

	auto mesh = meshRenderer->GetMesh();
	if (mesh == nullptr)
	{
		DEBUG->WarnLog("BoxCollider::FitOnMesh() - Mesh not found.");
		JPH::BoxShapeSettings shapeSettings(JPH::Vec3(1.0f, 1.0f, 1.0f));
		return shapeSettings.Create();
	}

	Bulb::Vector3 scale = GetTransform()->GetScale();
	float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
	float maxX = -FLT_MAX, maxY = -FLT_MAX, maxZ = -FLT_MAX;
	for (auto& v : mesh->GetVertices())
	{
		Bulb::Vector3 vertex = v.Position * scale;

		if (minX > vertex.x) minX = vertex.x;
		if (minY > vertex.y) minY = vertex.y;
		if (minZ > vertex.z) minZ = vertex.z;

		if (maxX < vertex.x) maxX = vertex.x;
		if (maxY < vertex.y) maxY = vertex.y;
		if (maxZ < vertex.z) maxZ = vertex.z;
	}

	_extents = Bulb::Vector3((maxX - minX) / 2, (maxY - minY) / 2, (maxZ - minZ) / 2);

	JPH::BoxShapeSettings shapeSettings(JPH::Vec3(_extents.x, _extents.y, _extents.z));
	return shapeSettings.Create();
}
