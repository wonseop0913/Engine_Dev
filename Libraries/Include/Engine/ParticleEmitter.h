#pragma once

#define MAX_PARTICLE_MOUNT	1024

struct BULB_API Particle {
	XMFLOAT3 Position;
	XMFLOAT3 Velocity;
	float Age;
	float Lifetime;
	UINT TextureIndex;
	XMFLOAT2 Size;
	XMFLOAT4 Color;
};

struct BULB_API EmitterSetting {
	Bulb::Vector3 EmitterPos;
	float ParticleInitialVelocity = 1;

	Bulb::Vector2 ParticleSize = { 1.0f, 1.0f };
	float GravityFactor = -9.8f;
	float ParticleLifeTime = 1;

	Bulb::Color InitialColor = { 1.0f, 1.0f, 1.0f, 1.0f };

	float EmitRate = 1.0f;		// Per second
	UINT EmitMount = 0;
	UINT StartIdx = 0;
	UINT TextureIdx = 0;

	Bulb::Vector3 EmitDirection = { 0.0f, 1.0f, 0.0f };
	float ConeAngle = 15.0f;

	UINT EmitterShape = 0;		// 0 = Default, 1 = Cone
};

enum class BULB_API EmitterShape {
	Sphere,
	Cone
};

class BULB_API ParticleEmitter : public Component
{
public:
	ParticleEmitter();
	virtual ~ParticleEmitter();

	void Init() override;
	void Update(ID3D12GraphicsCommandList* cmdList);	// 일반적인 Update와 다르게 CS를 위한 명령어 리스트를 필요로 함. override X
	void Render(ID3D12GraphicsCommandList* cmdList, UINT renderState) override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;
	void SaveXML(Bulb::XMLElement compElem) override;

	shared_ptr<Component> Duplicate() override;

	ComponentSnapshot CaptureSnapshot() override;
	void RestoreSnapshot(ComponentSnapshot snapshot) override;

public:
	bool IsPlaying() { return _isPlaying; }

	void SetPlay(bool value) { _isPlaying = value; }

	void SetParticleTexture(string textureName);
	void SetParticleTexture(wstring textureName);
	void SetParticleTexture(shared_ptr<Texture> texture);

	EmitterShape GetEmitterShape() { return _emitterShape; }
	void SetEmitterShape(EmitterShape shape) { _emitterShape = shape; }

	void SetMountPerTick(UINT value) { _mountPerTick = value; }

	Bulb::Vector3 GetConeDirection() { return _emitDirection; }
	void SetConeDirection(Bulb::Vector3& dir) { _emitDirection = dir; }

	float GetConeAngle() { return _emitterSetting.ConeAngle; }
	void SetConeAngle(float angle) { _emitterSetting.ConeAngle = angle; }

	EmitterSetting& GetParticleSetting() { return _emitterSetting; }
	void SetParticleSetting(EmitterSetting setting) { _emitterSetting = setting; }

	Bulb::Vector3 GetParticleOffset() { return _offset; }
	void SetParticleOffset(Bulb::Vector3& offset) { _offset = offset; }

private:
	ComPtr<ID3D12Resource> _particleBuffer;
	ComPtr<ID3D12Resource> _particleBufferUpload;

	EmitterShape _emitterShape;
	EmitterSetting _emitterSetting;
	Bulb::Vector3 _offset;
	Bulb::Vector3 _emitDirection = { 0.0f, 1.0f, 0.0f };
	string _particleTexture;

	float _instantiateTime = 0.0f;
	UINT _mountPerTick = 5;
	UINT _emitMount = 0;
	UINT _lastSpawnIdx = 0;
	bool _isPlaying = false;
};