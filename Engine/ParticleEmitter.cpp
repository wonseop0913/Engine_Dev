#include "pch.h"
#include "ParticleEmitter.h"

// REGISTER_COMPONENT(ParticleEmitter);

ParticleEmitter::ParticleEmitter() : Component(ComponentType::ParticleEmitter)
{

}

ParticleEmitter::~ParticleEmitter()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - ParticleEmitter:" << _id << "\n";
#endif
}

void ParticleEmitter::Init()
{
	int byteSize = sizeof(Particle) * MAX_PARTICLE_MOUNT;
	ThrowIfFailed(GRAPHIC->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&_particleBuffer)));

	PARTICLE->AddParticleEmitter(static_pointer_cast<ParticleEmitter>(shared_from_this()));
}

void ParticleEmitter::Update(ID3D12GraphicsCommandList* cmdList)
{
	_emitMount = 0;

	if (_isPlaying)  {
		_instantiateTime += TIME->DeltaTime();
		if (_instantiateTime >= _emitterSetting.EmitRate) {
			_instantiateTime = 0.0f;
			_emitMount += _mountPerTick;
		}
	}

	shared_ptr<Transform> transform = GetTransform();
	Bulb::Vector3 pos = transform->GetPosition();
	Bulb::Vector4 rot = transform->GetQuaternion();

	pos = pos + XMVector3Rotate(XMLoadFloat3(&_offset), XMLoadFloat4(&rot));

	_emitterSetting.EmitterPos = pos;
	_emitterSetting.EmitMount = _emitMount;
	_emitterSetting.StartIdx = _lastSpawnIdx;
	_emitterSetting.EmitterShape = (UINT)_emitterShape;

	if (_emitterShape == EmitterShape::Cone) {
		XMStoreFloat3(&_emitterSetting.EmitDirection, XMVector3Rotate(XMLoadFloat3(&_emitDirection), XMLoadFloat4(&rot)));
	}

	_lastSpawnIdx = (_lastSpawnIdx + _emitMount) % MAX_PARTICLE_MOUNT;

	cmdList->SetComputeRoot32BitConstants(ROOT_PARAM_EMITTER_CB, sizeof(EmitterSetting) / 4, &_emitterSetting, 0);

	cmdList->SetComputeRootUnorderedAccessView(ROOT_PARAM_PARTICLES_RW, _particleBuffer->GetGPUVirtualAddress());

	cmdList->Dispatch((MAX_PARTICLE_MOUNT + 255) / 256, 1, 1);
}

void ParticleEmitter::Render(ID3D12GraphicsCommandList* cmdList, UINT renderState)
{
	cmdList->SetGraphicsRootUnorderedAccessView(ROOT_PARAM_PARTICLES_RW, _particleBuffer->GetGPUVirtualAddress());
	cmdList->DrawInstanced(MAX_PARTICLE_MOUNT, 1, 0, 0);
}

void ParticleEmitter::OnDestroy()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "OnDestroy - ParticleEmitter:" << _id << "\n";
#endif

	PARTICLE->DeleteParticleEmitter(static_pointer_cast<ParticleEmitter>(shared_from_this()));
}

void ParticleEmitter::LoadXML(Bulb::XMLElement compElem)
{
	_isPlaying = compElem.BoolAttribute("IsPlaying");
	_mountPerTick = compElem.IntAttribute("MountPerTick", 5);
	_emitterSetting.EmitRate = compElem.FloatAttribute("SpawnRate", 1.0f);
	_emitterSetting.ParticleInitialVelocity = compElem.FloatAttribute("ParticleInitVelocity", 1.0f);
	_emitterSetting.GravityFactor = compElem.FloatAttribute("GravityFactor", -9.8f);
	_emitterSetting.ParticleLifeTime = compElem.FloatAttribute("ParticleLifeTime", 1.0f);
	_emitterShape = (EmitterShape)compElem.IntAttribute("EmitterShape");

	Bulb::XMLElement posOffsetElem = compElem.FirstChildElement("OffsetPosition");
	if (!posOffsetElem.IsNullPtr()) {
		_offset.x = posOffsetElem.FloatAttribute("X", 0.0f);
		_offset.y = posOffsetElem.FloatAttribute("Y", 0.0f);
		_offset.z = posOffsetElem.FloatAttribute("Z", 0.0f);
	}

	Bulb::XMLElement particleSizeElem = compElem.FirstChildElement("ParticleSize");
	if (!particleSizeElem.IsNullPtr()) {
		_emitterSetting.ParticleSize.x = particleSizeElem.FloatAttribute("X", 1.0f);
		_emitterSetting.ParticleSize.y = particleSizeElem.FloatAttribute("Y", 1.0f);
	}

	Bulb::XMLElement coneDirElem = compElem.FirstChildElement("ConeDirection");
	if (!coneDirElem.IsNullPtr()) {
		_emitDirection.x = coneDirElem.FloatAttribute("X", 0.0f);
		_emitDirection.y = coneDirElem.FloatAttribute("Y", 0.0f);
		_emitDirection.z = coneDirElem.FloatAttribute("Z", 0.0f);

		if (_emitDirection.Length() < 0.001f)
			_emitDirection = { 0.0f, 1.0f, 0.0f };

		_emitDirection = _emitDirection.Normalize();	// È¤½Ã ¸ô¶ó¼­ ³Ö¾îµÒ
	}

	_particleTexture = compElem.Attribute("ParticleTexture");
	_emitterSetting.TextureIdx = RESOURCE->Get<Texture>(Utils::ToWString(_particleTexture))->GetSRVHeapIndex();
}

void ParticleEmitter::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "ParticleEmitter");

	compElem.SetAttribute("IsPlaying", _isPlaying);
	compElem.SetAttribute("MountPerTick", (int)_mountPerTick);
	compElem.SetAttribute("SpawnRate", _emitterSetting.EmitRate);
	compElem.SetAttribute("ParticleInitVelocity", _emitterSetting.ParticleInitialVelocity);
	compElem.SetAttribute("GravityFactor", _emitterSetting.GravityFactor);
	compElem.SetAttribute("ParticleLifeTime", _emitterSetting.ParticleLifeTime);
	compElem.SetAttribute("EmitterShape", (int)_emitterShape);

	Bulb::XMLElement posOffsetElem = compElem.InsertNewChildElement("OffsetPosition");
	posOffsetElem.SetAttribute("X", _offset.x);
	posOffsetElem.SetAttribute("Y", _offset.y);
	posOffsetElem.SetAttribute("Z", _offset.z);

	Bulb::XMLElement particleSizeElem = compElem.InsertNewChildElement("ParticleSize");
	particleSizeElem.SetAttribute("X", _emitterSetting.ParticleSize.x);
	particleSizeElem.SetAttribute("Y", _emitterSetting.ParticleSize.y);

	Bulb::XMLElement coneDirElem = compElem.InsertNewChildElement("ConeDirection");
	coneDirElem.SetAttribute("X", _emitDirection.x);
	coneDirElem.SetAttribute("Y", _emitDirection.y);
	coneDirElem.SetAttribute("Z", _emitDirection.z);

	compElem.SetAttribute("ParticleTexture", _particleTexture.c_str());
}

shared_ptr<Component> ParticleEmitter::Duplicate()
{
	shared_ptr<ParticleEmitter> comp = static_pointer_cast<ParticleEmitter>(ComponentFactory::Create("ParticleEmitter"));

	comp->SetPlay(_isPlaying);
	comp->SetMountPerTick(_mountPerTick);
	comp->SetParticleSetting(_emitterSetting);
	comp->SetEmitterShape(_emitterShape);
	comp->SetParticleOffset(_offset);
	comp->SetConeDirection(_emitDirection);
	comp->SetParticleTexture(_particleTexture);

	return comp;
}

ComponentSnapshot ParticleEmitter::CaptureSnapshot()
{
	ComponentSnapshot snapshot;

	snapshot.id = _id;
	snapshot.componentType = "ParticleEmitter";

	return snapshot;
}

void ParticleEmitter::RestoreSnapshot(ComponentSnapshot snapshot)
{

}

void ParticleEmitter::SetParticleTexture(string textureName)
{
	SetParticleTexture(Utils::ToWString(textureName));
}

void ParticleEmitter::SetParticleTexture(wstring textureName)
{
	shared_ptr<Texture> tex = RESOURCE->Get<Texture>(textureName);
	if (tex == nullptr) {
		DEBUG->ErrorLog(Utils::ToString(textureName) + " does not exits!");
		return;
	}

	_emitterSetting.TextureIdx = tex->GetSRVHeapIndex();
}

void ParticleEmitter::SetParticleTexture(shared_ptr<Texture> texture)
{
	SetParticleTexture(texture->GetNameW());
}
