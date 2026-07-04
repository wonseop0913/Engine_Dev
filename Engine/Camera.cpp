#include "pch.h"
#include "Camera.h"

// REGISTER_COMPONENT(Camera);

shared_ptr<Camera> Camera::_currentCamera = nullptr;
shared_ptr<Camera> Camera::_editorCamera = nullptr;
UINT Camera::_cameraCount = 0;

Camera::Camera() : Super(ComponentType::Camera)
{
	_cameraCount++;
}

Camera::~Camera()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - Camera:" << _id << "\n";
#endif
}

void Camera::Init()
{
#ifdef BULB_EDITOR
	if (GetGameObject()->GetTag() == "EditorCamera")
		_editorCamera = static_pointer_cast<Camera>(shared_from_this());
#endif

	if (_isMainCamera)
		SetAsMainCamera();

	_aspectRatio = GRAPHIC->GetAspectRatio();
	_viewportSize = { GRAPHIC->GetViewport().Width, GRAPHIC->GetViewport().Height };

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, _aspectRatio, 1.0f, 1000.0f);
	XMStoreFloat4x4(&_matProj, P);

	XMMATRIX O = XMMatrixOrthographicLH(_viewportSize.x, _viewportSize.y, 0.0f, 1000.0f);
	XMStoreFloat4x4(&_matOrtho, O);
}

void Camera::Update()
{
	if (_blendTime > _elapsedBlendTime) {
		_elapsedBlendTime += TIME->DeltaTime();

		if (_elapsedBlendTime > _blendTime) {
			_colorBlend = _colorTarget;
			_blendTime = 0.0f;
			_elapsedBlendTime = 0.0f;
		}
	}

	if (GetGameObject()->GetFramesDirty() > 0) {
		XMVECTOR eyePos = XMLoadFloat3(&GetTransform()->GetPosition());
		XMVECTOR targetPos = eyePos + XMLoadFloat3(&GetTransform()->GetLook());
		XMVECTOR upVec = XMLoadFloat3(&GetTransform()->GetUp());

		// View
		XMMATRIX matView = XMMatrixLookAtLH(eyePos, targetPos, upVec);
		XMStoreFloat4x4(&_matView, matView);

		// Proj
		if (_aspectRatio != GRAPHIC->GetAspectRatio())
			_aspectRatio = GRAPHIC->GetAspectRatio();

		XMMATRIX matProj = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, _aspectRatio, _nearZ, _farZ);
		XMStoreFloat4x4(&_matProj, matProj);

		// View * Proj
		XMStoreFloat4x4(&_matViewProj, matView * XMLoadFloat4x4(&_matProj));
	}

	D3D12_VIEWPORT viewport = GRAPHIC->GetViewport();
	if (_viewportSize.x != viewport.Width || _viewportSize.y != viewport.Height) {
		XMMATRIX O = XMMatrixOrthographicLH(viewport.Width, viewport.Height, _nearZ, _farZ);
		XMStoreFloat4x4(&_matOrtho, O);
	}
}

bool Camera::ShowComponentEditorGUI()
{
	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::SeparatorText("Settings");
		ImGui::InputFloat("##CameraNearZ", &_nearZ);
		if (ImGui::IsItemDeactivatedAfterEdit())
			SetNearZ(_nearZ > 0.f ? _nearZ : 0.1f);
		ImGui::InputFloat("##CameraFarZ", &_farZ);
		if (ImGui::IsItemDeactivatedAfterEdit())
			SetFarZ(_farZ > _nearZ ? _farZ : _nearZ);

		ImGui::SeparatorText("Main Camera");
		if (ImGui::Button("Set Main Camera")) {
			SetAsMainCamera();
		}
		ImGui::Text("Is Main Camera: ");
		ImGui::SameLine();
		if (_isMainCamera)
			ImGui::TextColored({ 0, 1, 0, 1 }, "True");
		else
			ImGui::TextColored({ 1, 0, 0, 1 }, "False");
	}

	return false;
}

void Camera::OnDestroy()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "OnDestroy - Camera:" << _id << "\n";
#endif

	_currentCamera = nullptr;
}

void Camera::LoadXML(Bulb::XMLElement compElem)
{
	_nearZ = compElem.FloatAttribute("NearZ", 1.0f);
	_farZ = compElem.FloatAttribute("FarZ", 100.0f);

	_isMainCamera = compElem.BoolAttribute("MainCamera");
}

void Camera::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "Camera");
	compElem.SetAttribute("NearZ", _nearZ);
	compElem.SetAttribute("FarZ", _farZ);
	compElem.SetAttribute("MainCamera", _isMainCamera);
}

shared_ptr<Component> Camera::Duplicate()
{
	shared_ptr<Camera> comp = static_pointer_cast<Camera>(ComponentFactory::Create("Camera"));

	comp->SetNearZ(_nearZ);
	comp->SetFarZ(_farZ);
	// comp->SetAsMainCamera();

	return comp;
}

ComponentSnapshot Camera::CaptureSnapshot()
{
	ComponentSnapshot snapshot;

	snapshot.id = _id;
	snapshot.componentType = "Camera";

	snapshot.datas.push_back(IsMainCamera() ? 1 : 0);

	return snapshot;
}

void Camera::RestoreSnapshot(ComponentSnapshot snapshot)
{
	if (snapshot.datas[0] == 1)
		SetAsMainCamera();
}

shared_ptr<Camera> Camera::GetCurrentCamera()
{
#ifdef BULB_EDITOR
	return EDITOR->IsOnPlay() ? _currentCamera : _editorCamera;
#else
	return _currentCamera;
#endif
}

XMFLOAT3& Camera::GetEyePos()
{
#ifdef BULB_EDITOR
	if (EDITOR->IsOnPlay())
		return _currentCamera->GetTransform()->GetPosition();
	else
		return _editorCamera->GetTransform()->GetPosition();
#endif
	return _currentCamera->GetTransform()->GetPosition();
}

XMFLOAT4X4& Camera::GetViewMatrix()
{
#ifdef BULB_EDITOR
	if (EDITOR->IsOnPlay())
		return _currentCamera->_matView;
	else
		return _editorCamera->_matView;
#endif
	return _currentCamera->_matView;
}

XMFLOAT4X4& Camera::GetProjMatrix()
{
#ifdef BULB_EDITOR
	if (EDITOR->IsOnPlay())
		return _currentCamera->_matProj;
	else
		return _editorCamera->_matProj;
#endif
	return _currentCamera->_matProj;
}

XMFLOAT4X4& Camera::GetViewProjMatrix()
{
#ifdef BULB_EDITOR
	if (EDITOR->IsOnPlay())
		return _currentCamera->_matViewProj;
	else
		return _editorCamera->_matViewProj;
#endif
	return _currentCamera->_matViewProj;
}

XMFLOAT4X4& Camera::GetOrthoMatrix()
{
#ifdef BULB_EDITOR
	if (EDITOR->IsOnPlay())
		return _currentCamera->_matOrtho;
	else
		return _editorCamera->_matOrtho;
#endif
	return _currentCamera->_matOrtho;
}

int Camera::GetFramesDirty()
{
#ifdef BULB_EDITOR
	if (EDITOR->IsOnPlay())
		return _currentCamera->GetGameObject()->GetFramesDirty();
	else
		return _editorCamera->GetGameObject()->GetFramesDirty();
#endif
	return _currentCamera->GetGameObject()->GetFramesDirty();
}

CameraConstants Camera::GetCameraConstants()
{
	CameraConstants cameraConstants;
	XMMATRIX view = XMLoadFloat4x4(&_matView);
	XMMATRIX proj = XMLoadFloat4x4(&_matProj);
	XMMATRIX viewProj = XMLoadFloat4x4(&_matViewProj);

	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMMATRIX ortho = XMLoadFloat4x4(&_matOrtho);

	XMStoreFloat4x4(&cameraConstants.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&cameraConstants.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&cameraConstants.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&cameraConstants.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&cameraConstants.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&cameraConstants.InvViewProj, XMMatrixTranspose(invViewProj));
	XMStoreFloat4x4(&cameraConstants.Ortho, XMMatrixTranspose(ortho));

	AppDesc appDesc = GRAPHIC->GetAppDesc();

	cameraConstants.RenderTargetSize = XMFLOAT2((float)appDesc.clientWidth, (float)appDesc.clientHeight);
	cameraConstants.InvRenderTargetSize = XMFLOAT2(1.0f / appDesc.clientWidth, 1.0f / appDesc.clientHeight);

	if (_blendTime > 0.0f) {
		float blendRatio = _elapsedBlendTime / _blendTime;
		cameraConstants.CameraColorBlend = _colorBlend * (1.0f - blendRatio) + _colorTarget * blendRatio;
	}
	else
		cameraConstants.CameraColorBlend = _colorBlend;

	return cameraConstants;
}

void Camera::SetColorBlend(Bulb::Color color, float blendTime /*= 0.0f*/)
{
	_blendTime = blendTime;
	if (_blendTime <= 0.0f)
		_colorBlend = color;
	else {
		_colorTarget = color;
		_elapsedBlendTime = 0.0f;
	}
}

void Camera::SetAsMainCamera()
{
#ifdef BULB_EDITOR
	if (GetGameObject()->GetTag() == "EditorCamera") return;
#endif

	if (_currentCamera != nullptr) _currentCamera->_isMainCamera = false;
	_isMainCamera = true;
	_currentCamera = static_pointer_cast<Camera>(shared_from_this());
}
