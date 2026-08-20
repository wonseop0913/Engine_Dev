#include "pch.h"
#include "SoundManager.h"

SoundManager* SoundManager::s_instance = nullptr;

SoundManager::~SoundManager()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - SoundManager\n";
#endif

	_system->release();
}

SoundManager* SoundManager::GetInstance()
{
	if (s_instance == nullptr)
		s_instance = new SoundManager();
	return s_instance;
}

Bulb::ProcessResult SoundManager::Delete()
{
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
		return Bulb::ProcessResult::SUCCESS;
	}
	return Bulb::ProcessResult::FAILED_INSTANCE_NOT_FOUND;
}

void SoundManager::Init()
{
	FMOD::System_Create(&_system);
	_system->init(512, FMOD_INIT_NORMAL, 0);
	_system->set3DSettings(1.0f, 1.0f, 1.0f);

	_system->getMasterChannelGroup(&_masterGroup);
}

void SoundManager::Update()
{
	shared_ptr<Transform> cameraTransform = Camera::GetCurrentCamera()->GetTransform();
	Bulb::Vector3 listenerPos = cameraTransform->GetPosition();
	Bulb::Vector3 listenerForward = cameraTransform->GetLook();
	Bulb::Vector3 listenerUp = cameraTransform->GetUp();

	FMOD_VECTOR fListenerPos = { listenerPos.x, listenerPos.y, listenerPos.z };
	FMOD_VECTOR fListenerVel = { 0, 0, 0 };
	FMOD_VECTOR fListenerForward = { listenerForward.x, listenerForward.y, listenerForward.z };
	FMOD_VECTOR fListenerUp = { listenerUp.x, listenerUp.y, listenerUp.z };

	_system->set3DListenerAttributes(0, &fListenerPos, &fListenerVel, &fListenerForward, &fListenerUp);
	_system->update();
}

void SoundManager::StopAllSounds()
{
	_masterGroup->stop();
}

void SoundManager::StopSoundGroup(const string& group)
{
	_channelGroups[group]->stop();
}

FMOD::Sound* SoundManager::LoadSound(const string& path, bool loop)
{
	string fullPath = "../Resources/" + path;
	if (!filesystem::exists(fullPath)) return nullptr;

	FMOD_MODE mode = FMOD_3D;
	if (loop) mode = FMOD_LOOP_NORMAL;

	FMOD::Sound* sound = nullptr;
	FMOD_RESULT result = _system->createSound(fullPath.c_str(), mode, 0, &sound);

	// Legacy, 지워야함.
	// 단 현재 SoundManager를 직접적으로 사용해서 음원을 재생하는 기능을 바꾸고 나서
	_sounds[path] = sound;

	return sound;
}

void SoundManager::PlaySound(const string& name, const string& group)
{
	if (!_sounds.contains(name)) {
		DEBUG->ErrorLog("No sound loaded named '" + name +"'");
		return;
	}

	_system->playSound(_sounds[name], !_channelGroups.contains(group) ? _masterGroup : _channelGroups[group], false, nullptr);
}

void SoundManager::PlaySound(FMOD::Sound* sound, FMOD::Channel** channel)
{
	_system->playSound(sound, _masterGroup, false, channel);
}

void SoundManager::AddGroup(const string& name, const string& parentGroup)
{
	if (_channelGroups.contains(name)) return;

	FMOD::ChannelGroup* channelGroup;
	_system->createChannelGroup(name.c_str(), &channelGroup);
	_channelGroups[name] = channelGroup;

	if (_channelGroups.contains(parentGroup))
		_channelGroups[parentGroup]->addGroup(channelGroup);
	else
		_masterGroup->addGroup(channelGroup);
}

void SoundManager::SetMasterVolume(float value)
{
	_masterGroup->setVolume(value);
}

void SoundManager::SetVolume(float value, const string& group)
{
	_channelGroups[group]->setVolume(value);
}
