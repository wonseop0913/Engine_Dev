#pragma once
#include "Component.h"

class BULB_API AudioSource : public Component
{
public:
	AudioSource();
	~AudioSource();

	void Update() override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;

	void SaveXML(Bulb::XMLElement compElem) override;

	shared_ptr<Component> Duplicate() override;

	ComponentSnapshot CaptureSnapshot() override;

	void RestoreSnapshot(ComponentSnapshot snapshot) override;

#ifdef BULB_EDITOR
	bool ShowComponentEditorGUI() override;
#endif

public:
	void LoadSound(string path);

	void SetSound(FMOD::Sound* sound);

	void Play();

	void Stop();

	void SetVolume(float value);

	void SetLoop(bool value);

	void SetAsBGM(bool value);

	void Set3DMinMaxDistance(float min, float max) { _3dMinMaxDistance = { min, max }; }

	float Get3DMinDistance() { return _3dMinMaxDistance.x; }

	float Get3DMaxDistance() { return _3dMinMaxDistance.y; }

private:
	FMOD::Channel* _channel = nullptr;
	FMOD::Sound* _sound = nullptr;
	string _soundPath;
	FMOD_VECTOR _fPos;
	FMOD_VECTOR _fVelocity;

	Bulb::Vector2 _3dMinMaxDistance = { 10.0f, 100.0f };

	bool _isLoop = false;
	bool _isBGM = false;
	float _volume;
};

