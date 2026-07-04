#pragma once
#include "Component.h"

enum class LightType {
	Directional,
	Point
};

class Light : public Component
{
	using Super = Component;
public:
	Light();
	Light(XMFLOAT4 diffuse);
	virtual ~Light();

	virtual void Init() override = 0;
	virtual void Update() override = 0;

#ifdef BULB_EDITOR
	virtual bool ShowComponentEditorGUI() override = 0;
#endif

	virtual void OnDestroy() override = 0;

	virtual void LoadXML(Bulb::XMLElement compElem) override = 0;
	virtual void SaveXML(Bulb::XMLElement compElem) override = 0;

	virtual shared_ptr<Component> Duplicate() override = 0;

	virtual ComponentSnapshot CaptureSnapshot() override = 0;
	virtual void RestoreSnapshot(ComponentSnapshot snapshot) override = 0;

public:
	void SetFramesDirty();
	int GetFramesDirty() { return _numFramesDirty; }
	void ReleaseFramesDirty() { _numFramesDirty -= 1; }

	LightType GetLightType() { return _lightType; }

	XMFLOAT4X4& GetViewMatrix() { return _matView; }
	XMFLOAT4X4& GetProjMatrix() { return _matProj; }

	virtual LightConstants GetLightConstants() = 0;

public:
	Bulb::Color diffuse;
	float intensity = 1.0f;

protected:
	XMFLOAT4X4 _matView;
	XMFLOAT4X4 _matProj;

	LightType _lightType;

private:
	int _numFramesDirty = 0;
};

