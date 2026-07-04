#include "pch.h"
#include "Transform.h"

// REGISTER_COMPONENT(Transform);

Transform::Transform() : Super(ComponentType::Transform)
{
	_localPosition = { 0.0f, 0.0f, 0.0f };
	_localRotation = { 0.0f, 0.0f, 0.0f };
	_localScale = { 1.0f, 1.0f, 1.0f };

	_localQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f };
	_quaternion = { 0.0f, 0.0f, 0.0f, 1.0f };

	_parent = nullptr;

	_isDirty = true;

	_depthLevel = 0;
	UpdateTransform();
}

Transform::~Transform()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - Transfrom:" << _id << "\n";
#endif
}

bool Transform::ShowComponentEditorGUI()
{
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool isChanged = false;
		Bulb::Vector3 pos = GetLocalPosition();
		Bulb::Vector3 rot = GetLocalRotation();
		Bulb::Vector3 scale = GetLocalScale();

		ImGui::Text("Depth Level: %d", GetDepthLevel());

		ImGui::SeparatorText("Position");
		ImGui::Text("X");
		ImGui::SameLine();
		if (ImGui::InputFloat("##Transform_Position_X", &pos.x))
			isChanged = true;
		ImGui::Text("Y");
		ImGui::SameLine();
		if (ImGui::InputFloat("##Transform_Position_Y", &pos.y))
			isChanged = true;
		ImGui::Text("Z");
		ImGui::SameLine();
		if (ImGui::InputFloat("##Transform_Position_Z", &pos.z))
			isChanged = true;

		if (isChanged) {
			SetLocalPosition(pos);
		}

		isChanged = false;
		ImGui::SeparatorText("Rotation");
		ImGui::Text("X");
		ImGui::SameLine();
		if (ImGui::InputFloat("##Transform_Rotation_X", &rot.x))
			isChanged = true;
		ImGui::Text("Y");
		ImGui::SameLine();
		if (ImGui::InputFloat("##Transform_Rotation_Y", &rot.y))
			isChanged = true;
		ImGui::Text("Z");
		ImGui::SameLine();
		if (ImGui::InputFloat("##Transform_Rotation_Z", &rot.z))
			isChanged = true;

		if (isChanged)
			SetLocalRotation(rot);

		isChanged = false;
		ImGui::SeparatorText("Scale");
		ImGui::Text("X");
		ImGui::SameLine();
		if (ImGui::InputFloat("##Transform_Scale_X", &scale.x))
			isChanged = true;
		ImGui::Text("Y");
		ImGui::SameLine();
		if (ImGui::InputFloat("##Transform_Scale_Y", &scale.y))
			isChanged = true;
		ImGui::Text("Z");
		ImGui::SameLine();
		if (ImGui::InputFloat("##Transform_Scale_Z", &scale.z))
			isChanged = true;

		if (isChanged)
			SetLocalScale(scale);
	}

	return false;
}

void Transform::OnDestroy()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "OnDestroy - Transfrom:" << _id << "\n";
#endif
	if (_parent != nullptr) {
		_parent->RemoveChild(static_pointer_cast<Transform>(shared_from_this()));
	}
}

void Transform::LoadXML(Bulb::XMLElement compElem)
{
	// if (GetGameObject()->GetName() == "mixamorig:Sword_joint") __debugbreak();

	SetLocalPosition({ compElem.FloatAttribute("PosX"), compElem.FloatAttribute("PosY"), compElem.FloatAttribute("PosZ") });
	SetLocalScale({ compElem.FloatAttribute("ScaleX", 1.0f), compElem.FloatAttribute("ScaleY", 1.0f), compElem.FloatAttribute("ScaleZ", 1.0f) });
	//SetLocalRotation({ compElem.FloatAttribute("RotX"), compElem.FloatAttribute("RotY"), compElem.FloatAttribute("RotZ") });
	Bulb::Vector4 quat(compElem.FloatAttribute("QuatX", 0), compElem.FloatAttribute("QuatY", 0), compElem.FloatAttribute("QuatZ", 0), compElem.FloatAttribute("QuatW", 1));

	SetLocalQuaternion(quat.Normalize());

	// 수동으로 씬 작성하는게 아니면 굳이 필요할까 싶음
	//Bulb::XMLElement lookAtElem = compElem.FirstChildElement("LookAt");
	//if (lookAtElem) {
	//	LookAt({ lookAtElem->FloatAttribute("x"), lookAtElem->FloatAttribute("y"), lookAtElem->FloatAttribute("z") });
	//}
	//delete lookAtElem;
}

void Transform::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "Transform");

	// ForcedUpdateTransform() 쓰고 직접 접근하는건 문제가 없는지 검증 후 적용
	Bulb::Vector3 pos = GetLocalPosition();
	compElem.SetAttribute("PosX", pos.x);
	compElem.SetAttribute("PosY", pos.y);
	compElem.SetAttribute("PosZ", pos.z);

	Bulb::Vector3 scl = GetLocalScale();
	compElem.SetAttribute("ScaleX", scl.x);
	compElem.SetAttribute("ScaleY", scl.y);
	compElem.SetAttribute("ScaleZ", scl.z);

	//Vector3 rot = GetLocalRotation();
	//compElem.SetAttribute("RotX", rot.x);
	//compElem.SetAttribute("RotY", rot.y);
	//compElem.SetAttribute("RotZ", rot.z);

	Bulb::Vector4 quat = GetLocalQuaternion();
	compElem.SetAttribute("QuatX", quat.x);
	compElem.SetAttribute("QuatY", quat.y);
	compElem.SetAttribute("QuatZ", quat.z);
	compElem.SetAttribute("QuatW", quat.w);
}

shared_ptr<Component> Transform::Duplicate()
{
	shared_ptr<Transform> comp = static_pointer_cast<Transform>(ComponentFactory::Create("Transform"));

	comp->SetPosition(_position);
	comp->SetScale(_scale);
	comp->SetQuaternion(_quaternion);

	comp->ForceUpdateTransform();

	return comp;
}

ComponentSnapshot Transform::CaptureSnapshot()
{
	ComponentSnapshot snapshot;

	snapshot.id = _id;
	snapshot.componentType = "Transform";

	ForceUpdateTransform();
	Bulb::Vector3 pos = GetLocalPosition();
	snapshot.datas.push_back(pos.x);
	snapshot.datas.push_back(pos.y);
	snapshot.datas.push_back(pos.z);

	Bulb::Vector3 scl = GetLocalScale();
	snapshot.datas.push_back(scl.x);
	snapshot.datas.push_back(scl.y);
	snapshot.datas.push_back(scl.z);

	Bulb::Vector4 quat = GetLocalQuaternion();
	snapshot.datas.push_back(quat.x);
	snapshot.datas.push_back(quat.y);
	snapshot.datas.push_back(quat.z);
	snapshot.datas.push_back(quat.w);

	return snapshot;
}

void Transform::RestoreSnapshot(ComponentSnapshot snapshot)
{
	SetLocalPosition({ snapshot.datas[0], snapshot.datas[1], snapshot.datas[2] });
	SetLocalScale({ snapshot.datas[3], snapshot.datas[4], snapshot.datas[5] });
	SetLocalQuaternion(Bulb::Vector4(snapshot.datas[6], snapshot.datas[7], snapshot.datas[8], snapshot.datas[9]));
}

void Transform::ForceUpdateTransform()
{
	XMMATRIX matScale = XMMatrixScaling(_localScale.x, _localScale.y, _localScale.z);
	XMMATRIX matRotation = XMMatrixRotationQuaternion(XMLoadFloat4(&_localQuaternion));
	XMMATRIX matTranslation = XMMatrixTranslation(_localPosition.x, _localPosition.y, _localPosition.z);

	XMMATRIX matTransform = matScale * matRotation * matTranslation;

	XMStoreFloat4x4(&_matLocal, matTransform);

	if (HasParent())
	{
		XMMATRIX parentWorld = XMLoadFloat4x4(&_parent->GetWorldMatrix());
		XMStoreFloat4x4(&_matWorld, matTransform * parentWorld);
	}
	else
	{
		_matWorld = _matLocal;
	}

	_isDirty = false;

	XMVECTOR scale;
	XMVECTOR quaternion;
	XMVECTOR position;
	XMMatrixDecompose(
		&scale,
		&quaternion,
		&position,
		XMLoadFloat4x4(&_matWorld));

	XMStoreFloat3(&_scale, scale);
	_quaternion = quaternion;
	_rotation = MathHelper::ConvertQuaternionToEuler(quaternion);
	XMStoreFloat3(&_position, position);

	for (auto& child : _childs)
	{
		child->ForceUpdateTransform();
	}
}

void Transform::UpdateTransform()
{
	// 앞에서 조건 다 걸러내긴 하는데 혹시 몰라서 한번 더 확인
	if (!_isDirty)
		return;

	XMMATRIX matScale = XMMatrixScaling(_localScale.x, _localScale.y, _localScale.z);
	XMMATRIX matRotation = XMMatrixRotationQuaternion(XMLoadFloat4(&_localQuaternion));
	XMMATRIX matTranslation = XMMatrixTranslation(_localPosition.x, _localPosition.y, _localPosition.z);

	XMMATRIX matTransform = matScale * matRotation * matTranslation;

	XMStoreFloat4x4(&_matLocal, matTransform);

	if (HasParent())
	{
		XMMATRIX parentWorld = XMLoadFloat4x4(&_parent->GetWorldMatrix());
		XMStoreFloat4x4(&_matWorld, matTransform * parentWorld);
	}
	else
	{
		_matWorld = _matLocal;
	}

	_isDirty = false;

	XMVECTOR scale;
	XMVECTOR quaternion;
	XMVECTOR position;
	XMMatrixDecompose(
		&scale,
		&quaternion, 
		&position,
		XMLoadFloat4x4(&_matWorld));

	XMStoreFloat3(&_scale, scale);
	_quaternion = quaternion;
	_rotation = MathHelper::ConvertQuaternionToEuler(quaternion);
	XMStoreFloat3(&_position, position);

	for (auto& child : _childs)
	{
		child->SetDirtyFlag();
	}
}

void Transform::UpdateDepthLevel()
{
	_depthLevel = _parent->GetDepthLevel() + 1;
	for (auto& child : _childs)
		child->UpdateDepthLevel();
}

void Transform::SetLocalRotationRadian(const Bulb::Vector3& rotation)
{
	_localRotation = rotation;

	XMStoreFloat4(&_localQuaternion, XMQuaternionNormalize(XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)));

	SetDirtyFlag();
}

void Transform::SetQuaternion(const Bulb::Vector4& quaternion)
{
	XMVECTOR qLocal = XMQuaternionNormalize(XMLoadFloat4(&quaternion));
	_quaternion = qLocal;
	if (HasParent())
	{
		XMMATRIX parentRotMat = _parent->GetRotationMatrix();
		XMVECTOR qParent = XMQuaternionRotationMatrix(parentRotMat);
		qLocal = XMQuaternionMultiply(XMLoadFloat4(&quaternion), XMQuaternionInverse(qParent));
	}

	SetLocalQuaternion(qLocal);
}

void Transform::SetQuaternion(const XMVECTOR& quaternion)
{
	XMVECTOR qLocal = XMQuaternionNormalize(quaternion);
	_quaternion = qLocal;
	if (HasParent())
	{
		XMMATRIX parentRotMat = _parent->GetRotationMatrix();
		XMVECTOR qParent = XMQuaternionRotationMatrix(parentRotMat);
		qLocal = XMQuaternionMultiply(quaternion, XMQuaternionInverse(qParent));
	}

	SetLocalQuaternion(qLocal);
}

void Transform::SetLocalQuaternion(const Bulb::Vector4& quaternion)
{
	XMStoreFloat4(&_localQuaternion, XMQuaternionNormalize(XMLoadFloat4(&quaternion)));
	_localRotation = MathHelper::ConvertQuaternionToEuler(_localQuaternion);

	SetDirtyFlag();
}

void Transform::SetLocalQuaternion(const XMVECTOR& quaternion)
{
	XMStoreFloat4(&_localQuaternion, XMQuaternionNormalize(quaternion));
	_localRotation = MathHelper::ConvertQuaternionToEuler(_localQuaternion);

	SetDirtyFlag();
}

void Transform::SetPosition(const Bulb::Vector3& worldPosition)
{
	if (HasParent())
	{
		XMMATRIX inverseWorldMat = XMMatrixInverse(nullptr, XMLoadFloat4x4(&_parent->GetWorldMatrix()));
		Bulb::Vector3 position;
		XMStoreFloat3(&position, XMVector3Transform(XMLoadFloat3(&worldPosition), inverseWorldMat));

		SetLocalPosition(position);
	}
	else
	{
		SetLocalPosition(worldPosition);
	}
}

void Transform::SetRotationRadian(const Bulb::Vector3& worldRotation)
{
	XMVECTOR qWorld = XMQuaternionRotationRollPitchYaw(
		worldRotation.x,
		worldRotation.y,
		worldRotation.z);

	Bulb::Vector3 localEuler = worldRotation;
	if (HasParent())
	{
		XMMATRIX parentRotMat = _parent->GetRotationMatrix();
		XMVECTOR qParent = XMQuaternionRotationMatrix(parentRotMat);
		XMVECTOR qLocal = XMQuaternionMultiply(qWorld, XMQuaternionInverse(qParent));
		localEuler = MathHelper::ConvertQuaternionToEuler(qLocal);
	}

	SetLocalRotationRadian(localEuler);
}

void Transform::SetScale(const Bulb::Vector3& worldScale)
{
	if (HasParent())
	{
		Bulb::Vector3 parentScale = _parent->GetScale();
		Bulb::Vector3 scale = worldScale;
		scale.x /= parentScale.x;
		scale.y /= parentScale.y;
		scale.z /= parentScale.z;

		SetLocalScale(scale);
	}
	else
	{
		SetLocalScale(worldScale);
	}
}

XMMATRIX Transform::GetRotationMatrix()
{
	if (_isDirty)
		UpdateTransform();

	return XMMatrixRotationQuaternion(XMLoadFloat4(&_quaternion));
}

Bulb::Vector3 Transform::GetRight()
{
	if (_isDirty)
		UpdateTransform();

	return Bulb::Vector3(_matWorld._11, _matWorld._12, _matWorld._13).Normalize();
}

Bulb::Vector3 Transform::GetLeft()
{
	if (_isDirty)
		UpdateTransform();

	return Bulb::Vector3(-_matWorld._11, -_matWorld._12, -_matWorld._13).Normalize();
}

Bulb::Vector3 Transform::GetUp()
{
	if (_isDirty)
		UpdateTransform();

	return Bulb::Vector3(_matWorld._21, _matWorld._22, _matWorld._23).Normalize();
}

Bulb::Vector3 Transform::GetDown()
{
	if (_isDirty)
		UpdateTransform();

	return Bulb::Vector3(-_matWorld._21, -_matWorld._22, -_matWorld._23).Normalize();
}

Bulb::Vector3 Transform::GetLook()
{
	if (_isDirty)
		UpdateTransform();

	return Bulb::Vector3(_matWorld._31, _matWorld._32, _matWorld._33).Normalize();
}

Bulb::Vector3 Transform::GetBack()
{
	if (_isDirty)
		UpdateTransform();

	return Bulb::Vector3(-_matWorld._31, -_matWorld._32, -_matWorld._33).Normalize();
}

void Transform::Translate(const Bulb::Vector3& moveVec)
{
	_localPosition = _localPosition + moveVec;

	SetDirtyFlag();
}

void Transform::Rotate(const Bulb::Vector3& angle)
{
	XMVECTOR currentQuat = XMLoadFloat4(&_localQuaternion);
	XMVECTOR deltaQuat = XMQuaternionRotationRollPitchYaw(angle.x, angle.y, angle.z);
	XMVECTOR newQuat = XMQuaternionNormalize(XMQuaternionMultiply(currentQuat, deltaQuat));

	_localQuaternion = newQuat;
	_localRotation = MathHelper::ConvertQuaternionToEuler(newQuat);

	SetDirtyFlag();
}

void Transform::Rotate(const XMVECTOR& angle)
{
	Bulb::Vector3 f3_angle;
	XMStoreFloat3(&f3_angle, angle);
	Rotate(f3_angle);
}

void Transform::Rotate(const Bulb::Vector4& quat)
{
	XMVECTOR currentQuat = XMLoadFloat4(&_localQuaternion);
	XMVECTOR deltaQuat = XMLoadFloat4(&quat);
	XMVECTOR newQuat = XMQuaternionNormalize(XMQuaternionMultiply(currentQuat, deltaQuat));

	_localQuaternion = newQuat;
	_localRotation = MathHelper::ConvertQuaternionToEuler(newQuat);

	SetDirtyFlag();
}

void Transform::LookAt(const Bulb::Vector3& targetPos)
{
	XMVECTOR targetVec = XMVector3Normalize(XMLoadFloat3(&(targetPos - GetPosition())));

	XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR sideVec = XMVector3Normalize(XMVector3Cross(worldUp, targetVec));
	XMVECTOR upVec = XMVector3Normalize(XMVector3Cross(targetVec, sideVec));

	if (upVec.m128_f32[1] < 0.0f)
		upVec = -upVec;

	XMMATRIX rotMat = XMMatrixLookToLH(XMLoadFloat3(&GetPosition()), targetVec, upVec);

	rotMat = XMMatrixTranspose(rotMat);
	XMVECTOR quat = XMQuaternionNormalize(XMQuaternionRotationMatrix(rotMat));

	Bulb::Vector3 euler = MathHelper::ConvertQuaternionToEuler(quat);
	Bulb::Vector4 quatVec;
	XMStoreFloat4(&quatVec, quat);
	SetQuaternion(quatVec);
}

void Transform::LookAtWithNoRoll(const Bulb::Vector3& targetPos, float blendAlpha)
{
	Bulb::Vector3 dir = targetPos - GetPosition();
	dir = dir.Normalize();

	if (blendAlpha < 1.0f)
		dir = (dir * blendAlpha + GetLook() * (1.0f - blendAlpha)).Normalize();

	float pitch = -asin(dir.y);
	float yaw = atan2(dir.x, dir.z);

	XMVECTOR quatPitch = XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), pitch);
	XMVECTOR quatYaw = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), yaw);
	XMQuaternionRotationRollPitchYaw(pitch, yaw, 0.0f);
	XMVECTOR quatFinal = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0.0f);

	SetQuaternion(quatFinal);
}

void Transform::LookAtOnlyYaw(const Bulb::Vector3& targetPos, float blendAlpha)
{
	Bulb::Vector3 dir = targetPos - GetPosition();
	dir = dir.Normalize();

	if (blendAlpha < 1.0f)
		dir = (dir * blendAlpha + GetLook() * (1.0f - blendAlpha)).Normalize();

	float yaw = atan2(dir.x, dir.z);

	XMVECTOR quatYaw = XMQuaternionNormalize(XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), yaw));

	SetQuaternion(quatYaw);
}

void Transform::SetLocalMatrix(XMFLOAT4X4 mat)
{
	_matLocal = mat;

	XMVECTOR scale;
	XMVECTOR quaternion;
	XMVECTOR position;
	XMMatrixDecompose(
		&scale,
		&quaternion,
		&position,
		XMLoadFloat4x4(&_matLocal));

	XMStoreFloat3(&_localScale, scale);
	_localQuaternion = quaternion;
	_localRotation = MathHelper::ConvertQuaternionToEuler(quaternion);
	XMStoreFloat3(&_localPosition, position);

	SetDirtyFlag();
}

void Transform::SetLocalMatrix(XMMATRIX mat)
{
	XMFLOAT4X4 matTrans;
	XMStoreFloat4x4(&matTrans, mat);
	SetLocalMatrix(matTrans);
}

void Transform::SetWorldMatrix(XMFLOAT4X4 mat)
{
	_matWorld = mat;

	if (HasParent())
		XMStoreFloat4x4(&_matLocal, XMLoadFloat4x4(&_matWorld) * XMMatrixInverse(nullptr, XMLoadFloat4x4(&_parent->GetWorldMatrix())));
	else
		_matLocal = _matWorld;

	SetLocalMatrix(_matLocal);
}

XMFLOAT4X4 Transform::GetLocalMatrix()
{
	if (_isDirty)
		UpdateTransform();

	return _matLocal;
}

XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if (_isDirty)
		UpdateTransform();

	return _matWorld;
}

void Transform::SetParent(shared_ptr<Transform> parent)
{
	if (_parent != nullptr)
		_parent->RemoveChild(static_pointer_cast<Transform>(shared_from_this()));

	_parent = parent;

	if (_parent != nullptr)
		XMStoreFloat4x4(&_matLocal, XMLoadFloat4x4(&_matWorld) * XMMatrixInverse(nullptr, XMLoadFloat4x4(&_parent->GetWorldMatrix())));
	else
		_matLocal = _matWorld;

	_parent->AddChild(static_pointer_cast<Transform>(shared_from_this()));

	SetLocalMatrix(_matLocal);
}

void Transform::SetParentOnly(shared_ptr<Transform> parent)
{
	_parent = parent;

	_parent->AddChild(static_pointer_cast<Transform>(shared_from_this()));
	
	SetDirtyFlag();
}

shared_ptr<Transform> Transform::GetChild(const string& name)
{
	queue<shared_ptr<Transform>> childs;
	childs.push(static_pointer_cast<Transform>(shared_from_this()));
	while (!childs.empty()) {
		shared_ptr<Transform> current = childs.front();
		childs.pop();

		if (current->GetGameObject()->GetName() == name)
			return current;

		vector<shared_ptr<Transform>> subChilds = current->GetChilds();
		for (shared_ptr<Transform> c : subChilds)
			childs.push(c);
	}

	return nullptr;
}

void Transform::AddChild(shared_ptr<Transform> child)
{
	_childs.push_back(child);
	child->UpdateDepthLevel();
}

bool Transform::RemoveChild(shared_ptr<Transform> child)
{
	for (int i = 0; i < _childs.size(); ++i) {
		if (_childs[i] == child) {
			_childs.erase(_childs.begin() + i);
			return true;
		}
	}

	return false;
}

void Transform::SetDirtyFlag()
{
	_isDirty = true;
	GetGameObject()->SetFramesDirty();

	for (auto& child : _childs)
		child->SetDirtyFlag();
}
