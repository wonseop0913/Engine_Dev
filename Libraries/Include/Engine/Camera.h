#pragma once
#include "Component.h"

class BULB_API Camera : public Component
{
	friend class Camera;
	using Super = Component;
public:
	Camera();
	virtual ~Camera();

	void Init() override;
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
	static shared_ptr<Camera> GetCurrentCamera();

	static XMFLOAT3& GetEyePos();

	static XMFLOAT4X4& GetViewMatrix();

	static XMFLOAT4X4& GetProjMatrix();

	static XMFLOAT4X4& GetViewProjMatrix();

	static XMFLOAT4X4& GetOrthoMatrix();

	float GetNearZ() { return _nearZ; }
	void SetNearZ(float value) { _nearZ = value; }

	float GetFarZ() { return _farZ; }
	void SetFarZ(float value) { _farZ = value; }

	static int GetFramesDirty();

	CameraConstants GetCameraConstants();

	void SetColorBlend(Bulb::Color color, float blendTime = 0.0f);
	
	void SetAsMainCamera();
	bool IsMainCamera() { return _isMainCamera; }

private:
	XMFLOAT4X4 _matView;
	XMFLOAT4X4 _matProj;
	XMFLOAT4X4 _matViewProj;
	XMFLOAT4X4 _matOrtho;
	float _aspectRatio;
	float _nearZ = 1.0f;
	float _farZ = 100.0f;
	Bulb::Vector2 _viewportSize;

	bool _isMainCamera = false;
	static shared_ptr<Camera> _currentCamera;
	static UINT _cameraCount;

	float _blendTime = 0.0f;
	float _elapsedBlendTime = 0.0f;
	Bulb::Color _colorBlend = { 0.0f, 0.0f, 0.0f, 0.0f };
	Bulb::Color _colorTarget;

	static shared_ptr<Camera> _editorCamera;
};

