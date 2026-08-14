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

	_system->getMasterChannelGroup(&_masterGroup);
}

void SoundManager::Update()
{
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

void SoundManager::LoadSound(const string& path, bool loop)
{
	string fullPath = "../Resources/" + path;
	if (!filesystem::exists(fullPath)) return;

	FMOD_MODE mode = FMOD_DEFAULT;
	if (loop) mode = FMOD_LOOP_NORMAL;

	FMOD::Sound* sound = nullptr;
	FMOD_RESULT result = _system->createSound(fullPath.c_str(), mode, 0, &sound);

	_sounds[path] = sound;
}

void SoundManager::PlaySound(const string& name, const string& group)
{
	if (!_sounds.contains(name)) {
		DEBUG->ErrorLog("No sound loaded named '" + name +"'");
		return;
	}

	_system->playSound(_sounds[name], !_channelGroups.contains(group) ? _masterGroup : _channelGroups[group], false, nullptr);
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
