#pragma once

class BULB_API SoundManager
{
	friend class BulbApplication;

private:
	SoundManager() = default;
	~SoundManager();

public:
	SoundManager(const SoundManager& rhs) = delete;
	SoundManager& operator=(const SoundManager& rhs) = delete;

	static SoundManager* GetInstance();
	static Bulb::ProcessResult Delete();

	void Init();

	void Update(); 

	void StopAllSounds();

	void StopSoundGroup(const string& group);

	AudioClip LoadSound(const string& path, bool loop);

	void PlaySound(AudioClip sound, FMOD::Channel** channel = nullptr);

	void PlaySound(AudioClip sound, FMOD::Channel** channel, const string& group);

	void AddGroup(const string& name, const string& parentGroup = "");

	void SetMasterVolume(float value);

	void SetVolume(float value, const string& group);

private:
	static SoundManager* s_instance;

	FMOD::System* _system = nullptr;

	FMOD::ChannelGroup* _masterGroup = nullptr;

	unordered_map<string, FMOD::ChannelGroup*> _channelGroups;
};
