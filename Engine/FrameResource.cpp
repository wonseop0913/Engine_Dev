#include "pch.h"
#include "FrameResource.h"

FrameResource::FrameResource()
{
	for (int i = 0; i < 3; i++) {
		ThrowIfFailed(GRAPHIC->GetDevice()->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&cmdListAlloc[i])));
	}


	instanceSB = make_unique<UploadBuffer<InstanceConstants>>(DEFAULT_INSTANCE_COUNT, false);
	materialSB = make_unique<UploadBuffer<MaterialConstants>>(DEFAULT_MATERIAL_COUNT, false);
	lightSB = make_unique<UploadBuffer<LightConstants>>(DEFAULT_COUNT_LIGHT, false);

	cameraCB = make_unique<UploadBuffer<CameraConstants>>(1, true);
}

FrameResource::~FrameResource()
{

}

void FrameResource::Init()
{
	BuildInstanceBufferSRV();
	BuildMaterialBufferSRV();
	BuildLightBufferSRV();
}

void FrameResource::Update()
{
	if (!_isInitialized)
	{
		Init();
		_isInitialized = true;
	}

	// 순서 바꾸면 안됨!!!
	_futures[0] = THREAD->EnqueueJob([this] { UpdateObjectSB(); });
	_futures[1] = THREAD->EnqueueJob([this] { UpdateCameraCB(); });
	_futures[2] = THREAD->EnqueueJob([this] { UpdateLightSB(); });

	UpdateMaterialSB();

	_futures[0].get();
	_futures[1].get();
	_futures[2].get();
}

void FrameResource::UpdateObjectSB()
{
	auto objects = RENDER->GetObjects();
	for (auto& o : objects)
	{
		int instanceIndex = 0;

		shared_ptr<MeshRenderer> meshRenderer = o->GetComponent<MeshRenderer>();
		if (meshRenderer == nullptr) 
			meshRenderer = o->GetComponent<SkinnedMeshRenderer>();

		if (meshRenderer != nullptr) {
			shared_ptr<Mesh> mesh = meshRenderer->GetMesh();
			if (mesh != nullptr) {
				instanceIndex = RENDER->GetMeshInstanceStartIndex(mesh);

				UINT meshId = mesh->GetID();
				if (_instanceIndices.size() > meshId) {
					instanceIndex += _instanceIndices[meshId];
					++_instanceIndices[meshId];
				}
				else {
					_instanceIndices.insert(_instanceIndices.end(), meshId - _instanceIndices.size() + 1, 0);
					_instanceIndices[meshId] = 1;
				}
			}
		}

		if (o->GetFramesDirty() > 0)
		{
			o->ReleaseFramesDirty();

			if (meshRenderer != nullptr) {
				shared_ptr<Material> mat = meshRenderer->GetMaterial();
				if (mat != nullptr) {
					InstanceConstants instanceConstants;

					instanceConstants.MaterialIndex = mat->matSBIndex;

					XMMATRIX world = XMLoadFloat4x4(&o->GetTransform()->GetWorldMatrix());
					XMStoreFloat4x4(&instanceConstants.World, XMMatrixTranspose(world));
					XMStoreFloat4x4(&instanceConstants.WorldInv, XMMatrixTranspose(XMMatrixInverse(nullptr, world)));

					instanceSB->CopyData(instanceIndex, instanceConstants);
				}
			}
		}
	}

	_instanceIndices.assign(_instanceIndices.size(), 0);
}

void FrameResource::UpdateMaterialSB()
{
	auto materials = RESOURCE->GetByType<Material>();
	for (auto& m : materials)
	{
		shared_ptr<Material> mat = static_pointer_cast<Material>(m.second);
		if (mat->numFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->matTransform);

			MaterialConstants matConstants;
			XMStoreFloat4x4(&matConstants.matTransform, XMMatrixTranspose(matTransform));
			matConstants.Ambient = mat->ambient;
			matConstants.Diffuse = mat->diffuse;
			matConstants.Specular = mat->specular;
			matConstants.Emissive = mat->emissive;
			matConstants.Tilling = mat->tilling;
			matConstants.Metallic = mat->metallic;
			matConstants.Roughness = mat->roughness;
			matConstants.DiffuseMapIndex = mat->diffuseSrvHeapIndex;
			matConstants.NormalMapIndex = mat->normalSrvHeapIndex;
			matConstants.MetallicMapIndex = mat->metallicSrvHeapIndex;
			matConstants.RoughnessMapIndex = mat->roughnessSrvHeapIndex;
			matConstants.SpecularMapIndex = mat->specularSrvHeapIndex;

			materialSB->CopyData(mat->matSBIndex, matConstants);

			mat->numFramesDirty--;
		}
	}
}

void FrameResource::UpdateLightSB()
{
	auto lights = RENDER->GetLights();

	if (lights.size() == 0)
		return;

	for (int i = 0; i < lights.size(); i++)
	{
		if (lights[i]->GetFramesDirty() <= 0)
			continue;
		
		lightSB->CopyData(i, lights[i]->GetLightConstants());
	}
}

void FrameResource::UpdateCameraCB()
{
	cameraCB->CopyData(0, Camera::GetCurrentCamera()->GetCameraConstants());
}

void FrameResource::BuildInstanceBufferSRV()
{
	_instanceSrvHeapIndex = RENDER->GetAndIncreaseSRVHeapIndex();
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(RENDER->GetCommonSRVHeap()->GetCPUDescriptorHandleForHeapStart());
	hDescriptor.Offset(_instanceSrvHeapIndex, GRAPHIC->GetCBVSRVDescriptorSize());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = DEFAULT_INSTANCE_COUNT;
	srvDesc.Buffer.StructureByteStride = sizeof(InstanceConstants);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	GRAPHIC->GetDevice()->CreateShaderResourceView(instanceSB->GetResource(), &srvDesc, hDescriptor);
}

void FrameResource::BuildMaterialBufferSRV()
{
	_materialSrvHeapIndex = RENDER->GetAndIncreaseSRVHeapIndex();
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(RENDER->GetCommonSRVHeap()->GetCPUDescriptorHandleForHeapStart());
	hDescriptor.Offset(_materialSrvHeapIndex, GRAPHIC->GetCBVSRVDescriptorSize());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = DEFAULT_MATERIAL_COUNT;
	srvDesc.Buffer.StructureByteStride = sizeof(MaterialConstants);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	GRAPHIC->GetDevice()->CreateShaderResourceView(materialSB->GetResource(), &srvDesc, hDescriptor);
}

void FrameResource::BuildLightBufferSRV()
{
	auto lights = RENDER->GetLights();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = DEFAULT_COUNT_LIGHT;
	srvDesc.Buffer.StructureByteStride = sizeof(LightConstants);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	_lightSrvHeapIndex = RENDER->GetAndIncreaseSRVHeapIndex();
	CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle(RENDER->GetCommonSRVHeap()->GetCPUDescriptorHandleForHeapStart());
	heapHandle.Offset(_lightSrvHeapIndex, GRAPHIC->GetCBVSRVDescriptorSize());

	GRAPHIC->GetDevice()->CreateShaderResourceView(lightSB->GetResource(), &srvDesc, heapHandle);
}
