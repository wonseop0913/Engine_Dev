#pragma once

class BULB_API EditorCamera : public Script
{
public:
	~EditorCamera();

	void Init() override;
	void Update() override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override {}
	void SaveXML(Bulb::XMLElement compElem) override {}

	void MoveToTargetObject(shared_ptr<Transform> target);

public:
	shared_ptr<Transform> transform;
	float yaw = 0.0f;
	float pitch = 0.0f;
	float shiftSpeed = 2.5f;
};

