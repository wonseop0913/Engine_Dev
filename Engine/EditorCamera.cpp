#include "pch.h"
#include "EditorCamera.h"

// REGISTER_COMPONENT(EditorCamera)

EditorCamera::~EditorCamera()
{

}

void EditorCamera::Init()
{
	transform = GetTransform();
}

void EditorCamera::Update()
{
	if (EDITOR->IsOnPlay()) return;

	if (EDITOR->isNoneMouseMode) {
		float cameraSpeed = INPUTM->IsKeyPress(KeyValue::SHIFT) ? shiftSpeed : 1.0f;

		// Mouse Delta를 사용하지 않고 키보드 화살표로 각도조절할 수 있도록
		if (INPUTM->IsKeyPress(KeyValue::UP)) {
			pitch -= TIME->DeltaTime() * cameraSpeed;
		}
		if (INPUTM->IsKeyPress(KeyValue::DOWN)) {
			pitch += TIME->DeltaTime() * cameraSpeed;
		}
		if (INPUTM->IsKeyPress(KeyValue::RIGHT)) {
			yaw += TIME->DeltaTime() * cameraSpeed;
		}
		if (INPUTM->IsKeyPress(KeyValue::LEFT)) {
			yaw -= TIME->DeltaTime() * cameraSpeed;
		}

		XMVECTOR quatPitch = XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), pitch);
		XMVECTOR quatYaw = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), yaw);

		XMVECTOR quatFinal = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0.0f);

		transform->SetQuaternion(quatFinal);

		if (INPUTM->IsKeyPress(KeyValue::W))
			transform->Translate(transform->GetLook() * TIME->DeltaTime() * 5.0f);
		if (INPUTM->IsKeyPress(KeyValue::S))
			transform->Translate(transform->GetLook() * TIME->DeltaTime() * -5.0f);
		if (INPUTM->IsKeyPress(KeyValue::D))
			transform->Translate(transform->GetRight() * TIME->DeltaTime() * 5.0f);
		if (INPUTM->IsKeyPress(KeyValue::A))
			transform->Translate(transform->GetRight() * TIME->DeltaTime() * -5.0f);
		if (INPUTM->IsKeyPress(KeyValue::E))
			transform->Translate(transform->GetUp() * TIME->DeltaTime() * 5.0f);
		if (INPUTM->IsKeyPress(KeyValue::Q))
			transform->Translate(transform->GetUp() * TIME->DeltaTime() * -5.0f);
	}
	else {
		if (INPUTM->IsMouseRightButtonPress()) {
			Bulb::Vector2 mouseDelta = INPUTM->GetMouseDelta();

			yaw += mouseDelta.x * TIME->DeltaTime();
			pitch += mouseDelta.y * TIME->DeltaTime();

			XMVECTOR quatPitch = XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), pitch);
			XMVECTOR quatYaw = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), yaw);

			XMVECTOR quatFinal = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0.0f);

			transform->SetQuaternion(quatFinal);

			if (INPUTM->IsKeyPress(KeyValue::W))
				transform->Translate(transform->GetLook() * TIME->DeltaTime() * 5.0f);
			if (INPUTM->IsKeyPress(KeyValue::S))
				transform->Translate(transform->GetLook() * TIME->DeltaTime() * -5.0f);
			if (INPUTM->IsKeyPress(KeyValue::D))
				transform->Translate(transform->GetRight() * TIME->DeltaTime() * 5.0f);
			if (INPUTM->IsKeyPress(KeyValue::A))
				transform->Translate(transform->GetRight() * TIME->DeltaTime() * -5.0f);
			if (INPUTM->IsKeyPress(KeyValue::E))
				transform->Translate(transform->GetUp() * TIME->DeltaTime() * 5.0f);
			if (INPUTM->IsKeyPress(KeyValue::Q))
				transform->Translate(transform->GetUp() * TIME->DeltaTime() * -5.0f);
		}
	}
}

void EditorCamera::OnDestroy()
{
	transform.reset();
}

void EditorCamera::MoveToTargetObject(shared_ptr<Transform> target)
{
	Bulb::Vector3 targetPosDir = -transform->GetLook();
	transform->SetPosition(target->GetPosition() + targetPosDir * 5.0f);
}
