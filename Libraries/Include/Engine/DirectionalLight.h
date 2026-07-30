#pragma once
#include "Light.h"

class BULB_API DirectionalLight : public Light
{
	using Super = Light;
public:
	DirectionalLight();
	DirectionalLight(XMFLOAT4 diffuse);
	virtual ~DirectionalLight();

	void Init() override;
	void Update() override;

	void OnDestroy() override;

	void SetActive(bool value) override;

	void LoadXML(Bulb::XMLElement compElem) override;
	void SaveXML(Bulb::XMLElement compElem) override;

	shared_ptr<Component> Duplicate() override;

	ComponentSnapshot CaptureSnapshot() override;
	void RestoreSnapshot(ComponentSnapshot snapshot) override;

#ifdef BULB_EDITOR
	bool ShowComponentEditorGUI() override;
#endif

public:
	LightConstants GetLightConstants() override;

public:
	Bulb::Vector3 direction = { 0.0f, 1.0f, 0.0f };

private:
	shared_ptr<Transform> _transform;
};

