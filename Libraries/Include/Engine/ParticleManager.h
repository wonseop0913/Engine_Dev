#pragma once

class BULB_API ParticleManager
{
	friend class BulbApplication;
	friend class RenderManager;

private:
	ParticleManager() = default;
	~ParticleManager();

	void Init();
	void Update(ID3D12GraphicsCommandList* cmdList);
	void Render(ID3D12GraphicsCommandList* cmdList, UINT renderState);

public:
	ParticleManager(const ParticleManager& rhs) = delete;
	ParticleManager& operator=(const ParticleManager& rhs) = delete;

	static ParticleManager* GetInstance();
	static Bulb::ProcessResult Delete();

	void AddParticleEmitter(shared_ptr<ParticleEmitter> particleEmitter);
	void DeleteParticleEmitter(shared_ptr<ParticleEmitter> particleEmitter);

private:
	static ParticleManager* s_instance;

	// map�� ����ϴ°� �ּ����� ����
	unordered_map<int, shared_ptr<ParticleEmitter>> _particleEmitters;
};

