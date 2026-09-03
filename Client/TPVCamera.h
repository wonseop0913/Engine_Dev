// Third Person View Camera Script

#pragma once

class TPVCamera : public Script
{
public:
	~TPVCamera();

	void Init() override;
	void Update() override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;
	void SaveXML(Bulb::XMLElement compElem) override;

public:
	shared_ptr<Transform> armTransform;
	shared_ptr<Transform> cameraTransform;
	shared_ptr<Transform> onwerTransform;
	shared_ptr<Transform> lockOnTargetTransform;

	float distance = 3.5f;
	Bulb::Vector3 offset = { 0.0f, 0.0f, 0.0f };
	float sensitivity = 1.0f;
	Bulb::Vector2 pitchLimit = { 80.0f, -80.0f };
	float rotationSharpness = 0.3f;			// range : 0.1f ~ 1.0f, ���� �����ؼ� ����ؾ���.
	float pivotMovementSharpness = 6.0f;	// range : 1.0f ~ 10.0f

	// ī�޶� ������ ���� ����
	bool isCameraControllOn = true;
	bool isLockOn = false;

	string initTargetTag = "Player";

private:
	string _armTransformName;
	string _cameraTransformName;

	shared_ptr<Transform> _transform;
	Bulb::Vector3 _pivotPosition;
	Bulb::Vector3 _targetPivotPosition;
	float _pitch = 0.0f;

	shared_ptr<UIPanel> _lockOnMarker = nullptr;
};

