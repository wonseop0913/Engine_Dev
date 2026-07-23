#include "pch.h"
#include "CharacterController.h"

// REGISTER_COMPONENT(CharacterController);

CharacterController::CharacterController() : Component(ComponentType::CharacterController)
{

}

CharacterController::~CharacterController()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - CharacterController:" << _id << "\n";
#endif
}

void CharacterController::Init()
{
	JPH::CapsuleShapeSettings shapeSettings;
	shapeSettings.mHalfHeightOfCylinder = _height;
	shapeSettings.mRadius = _radius;
	_shape = shapeSettings.Create().Get();

	JPH::CharacterVirtualSettings characterSettings;
	characterSettings.mShape = _shape;
	characterSettings.mMaxSlopeAngle = JPH::DegreesToRadians(45.0f);
	characterSettings.mMaxStrength = 100.0f;
	characterSettings.mCharacterPadding = 0.02f;
	characterSettings.mMass = 80.0f;
	characterSettings.mMaxStrength = 1000.0f;

	Bulb::Vector3 pos = GetTransform()->GetPosition();

	_character = new JPH::CharacterVirtual(
		&characterSettings,
		JPH::RVec3(pos.x, pos.y, pos.z),
		JPH::Quat::sIdentity(),
		PHYSICS->GetPhysicsSystem()
	);

	_bodyId = _character->GetInnerBodyID();
	_character->SetShapeOffset(JPH::Vec3Arg(_offset.x, _offset.y, _offset.z));
	_character->SetUserData(reinterpret_cast<JPH::uint64>(_gameObject.lock().get()));

	_character->SetListener(PHYSICS);
}

void CharacterController::PreUpdate()
{
#ifdef BULB_EDITOR
	if (!EDITOR->IsOnPlay()) return;
#endif

	Bulb::Vector3 pos = GetTransform()->GetPosition();
	_character->SetPosition({ pos.x, pos.y, pos.z });

	// 중력이 두 번 적용되고 있는건 아닌지?
	_verticalVelocity -=  GRAVITY_ACC * TIME->DeltaTime() * (_isGravity ? _gravityFactor : 0.0f);
	_currentVelocity = _desiredVelocity;
	_currentVelocity.y += _verticalVelocity;
	JPH::Vec3 totalVelocity(_currentVelocity.x, _currentVelocity.y, _currentVelocity.z);
	_character->SetLinearVelocity(totalVelocity);

	JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings;
	updateSettings.mStickToFloorStepDown = JPH::Vec3(0.0f, -0.5f, 0.0f);
	updateSettings.mWalkStairsStepUp = JPH::Vec3(0.0f, 0.4f, 0.0f);

	_character->ExtendedUpdate(
		TIME->DeltaTime(),
		-_character->GetUp() * GRAVITY_ACC * (_isGravity ? _gravityFactor : 0.0f),
		updateSettings,
		PHYSICS->GetPhysicsSystem()->GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
		PHYSICS->GetPhysicsSystem()->GetDefaultLayerFilter(Layers::MOVING),
		{ }, {}, *PHYSICS->GetTempAllocator()
	);

	JPH::Vec3 newPos = _character->GetPosition();
	GetTransform()->SetPosition(Bulb::Vector3(newPos.GetX(), newPos.GetY(), newPos.GetZ()));

	if (_character->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround)
		_verticalVelocity = 0.0f;


	_desiredVelocity = { 0.0f, 0.0f, 0.0f };
}

bool CharacterController::ShowComponentEditorGUI()
{
	if (ImGui::CollapsingHeader("CharacterController", ImGuiTreeNodeFlags_DefaultOpen)) {

	}

	return false;
}

void CharacterController::OnDestroy()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "OnDestroy - CharacterController:" << _id << "\n";
#endif
}

void CharacterController::LoadXML(Bulb::XMLElement compElem)
{

}

void CharacterController::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "CharacterController");
}

shared_ptr<Component> CharacterController::Duplicate()
{
	shared_ptr<CharacterController> comp = static_pointer_cast<CharacterController>(ComponentFactory::Create("CharacterController"));

	return comp;
}

ComponentSnapshot CharacterController::CaptureSnapshot()
{
	ComponentSnapshot snapshot;

	snapshot.id = _id;
	snapshot.componentType = "CharacterController";

	snapshot.datas.push_back(_isGravity ? 1 : 0);

	return snapshot;
}

void CharacterController::RestoreSnapshot(ComponentSnapshot snapshot)
{
	SetGravity(snapshot.datas[0] == 1);

	_currentVelocity = { 0.0f, 0.0f, 0.0f };
	_desiredVelocity = { 0.0f, 0.0f, 0.0f };
	_verticalVelocity = 0.0f;
}

bool CharacterController::IsOnGround()
{
	JPH::CharacterBase::EGroundState groundState = _character->GetGroundState();
	return groundState == JPH::CharacterBase::EGroundState::OnGround || groundState == JPH::CharacterBase::EGroundState::OnSteepGround;
}
