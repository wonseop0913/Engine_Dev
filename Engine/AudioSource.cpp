#include "pch.h"
#include "AudioSource.h"

AudioSource::AudioSource() : Component(ComponentType::AudioSource)
{

}

AudioSource::~AudioSource()
{

}

void AudioSource::Update()
{
	if (_channel != nullptr && !_isBGM) {
		Bulb::Vector3 bPos = GetTransform()->GetPosition();
		_fVelocity = { bPos.x - _fPos.x, bPos.y - _fPos.y, bPos.z - _fPos.z };
		_fPos = { bPos.x, bPos.y, bPos.z };
		_channel->set3DMinMaxDistance(_3dMinMaxDistance.x, _3dMinMaxDistance.y);
		_channel->set3DAttributes(&_fPos, &_fVelocity);
	}
}

void AudioSource::OnDestroy()
{

}

void AudioSource::LoadXML(Bulb::XMLElement compElem)
{
	const char* filePath = compElem.Attribute("FilePath");
	if (filePath != 0) {
		_soundPath = filePath;
		LoadSound(_soundPath);
	}

	_isLoop = compElem.BoolAttribute("Loop");
	if (_sound != nullptr) {
		SetLoop(_isLoop);
	}

	_isBGM = compElem.BoolAttribute("BGM");
	if (_sound != nullptr) {
		SetAsBGM(_isBGM);
	}
}

void AudioSource::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "AudioSource");

	compElem.SetAttribute("FilePath", _soundPath.c_str());
	compElem.SetAttribute("Loop", _isLoop);
	compElem.SetAttribute("BGM", _isBGM);
}

std::shared_ptr<Component> AudioSource::Duplicate()
{
	shared_ptr<Component> comp = static_pointer_cast<Component>(ComponentFactory::Create("Component"));

	return comp;
}

ComponentSnapshot AudioSource::CaptureSnapshot()
{
	ComponentSnapshot snapshot;

	snapshot.id = _id;
	snapshot.componentType = "AudioSource";

	return snapshot;
}

void AudioSource::RestoreSnapshot(ComponentSnapshot snapshot)
{

}

bool AudioSource::ShowComponentEditorGUI()
{
	if (ImGui::CollapsingHeader("AudioSource", ImGuiTreeNodeFlags_DefaultOpen)) {

	}

	return false;
}

void AudioSource::LoadSound(string path)
{
	_soundPath = path;
	_sound = SOUND->LoadSound(_soundPath, false);
}

void AudioSource::SetSound(FMOD::Sound* sound)
{
	if (sound == nullptr) return;

	_sound = sound;
	char soundPath[256];
	sound->getName(soundPath, sizeof(soundPath));
	_soundPath = soundPath;
}

void AudioSource::Play()
{
	SOUND->PlaySound(_sound, &_channel);
}

void AudioSource::Stop()
{
	_channel->stop();
}

bool AudioSource::IsPlaying()
{
	if (_channel == nullptr) false;

	bool flag;
	_channel->isPlaying(&flag);
	return flag;
}

void AudioSource::SetVolume(float value)
{
	_channel->setVolume(value);
}

void AudioSource::SetLoop(bool value)
{
	_isLoop = value;

	if (_isLoop) {
		_isBGM ? _sound->setMode(FMOD_2D | FMOD_LOOP_NORMAL) : _sound->setMode(FMOD_3D | FMOD_LOOP_NORMAL);
	}
	else {
		_isBGM ? _sound->setMode(FMOD_2D) : _sound->setMode(FMOD_3D);
	}
}

void AudioSource::SetAsBGM(bool value)
{
	_isBGM = value;

	if (_isLoop) {
		_isBGM ? _sound->setMode(FMOD_2D | FMOD_LOOP_NORMAL) : _sound->setMode(FMOD_3D | FMOD_LOOP_NORMAL);
	}
	else {
		_isBGM ? _sound->setMode(FMOD_2D) : _sound->setMode(FMOD_3D);
	}
}
