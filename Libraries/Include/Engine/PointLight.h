#pragma once
#include "Light.h"

class BULB_API PointLight : public Light
{
	using Super = Light;
public:
	PointLight();
	PointLight(XMFLOAT4 diffuse);
	virtual ~PointLight();

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

	Bulb::Vector2 GetFallOffValues() { return { _fallOffStart, _fallOffEnd }; }
	
	float GetFallOffStart() { return _fallOffStart; }
	
	float GetFallOffEnd() { return _fallOffEnd; }
	
	void SetFallOffValues(float start, float end) { _fallOffStart = start; _fallOffEnd = end; }

	void SetFallOffValues(Bulb::Vector2 values) { _fallOffStart = values.x; _fallOffEnd = values.y; }

public:
	LightConstants GetLightConstants() override;
	
private:
	shared_ptr<Transform> _transform;
	Bulb::Vector3 _position;

	float _fallOffStart = 1.0f;
	float _fallOffEnd = 2.0f;
};

