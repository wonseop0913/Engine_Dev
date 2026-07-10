#include "pch.h"
#include "Skeleton.h"

Skeleton::Skeleton()
{

}

Skeleton::~Skeleton()
{

}

void Skeleton::Init()
{

}

void Skeleton::UpdateUploadBuffer()
{
	_boneTransformUploadBuffer->CopyData(_finalMatrices.data(), _bones.size());
}

void Skeleton::UpdateBoneTransform(int id, XMMATRIX localMat)
{
	instancedTransforms[id]->SetLocalMatrix(localMat);
	XMStoreFloat4x4(&_finalMatrices[id], XMMatrixTranspose(XMLoadFloat4x4(&_bones[id].offsetTransform) * XMLoadFloat4x4(&instancedTransforms[id]->GetWorldMatrix())));
}

void Skeleton::UpdateBoneTransform(int id)
{
	XMStoreFloat4x4(&_finalMatrices[id], XMMatrixTranspose(XMLoadFloat4x4(&_bones[id].offsetTransform) * XMLoadFloat4x4(&instancedTransforms[id]->GetWorldMatrix())));
}

void Skeleton::SetBone(map<string, BoneData> bones)
{
	_bones.clear();
	_bones.resize(bones.size(), BoneData());
	instancedTransforms.clear();
	instancedTransforms.resize(bones.size(), nullptr);
	_finalMatrices.clear();
	_finalMatrices.resize(bones.size(), XMFLOAT4X4());

	for (auto b : bones)
		_bones[b.second.id] = b.second;

	CreateBuffer();
}

void Skeleton::MapTransform(shared_ptr<Transform> transform)
{
	int depthOffset = -1;
	depthSortedTransformIndices.clear();

	vector<shared_ptr<Transform>> childs;
	childs.push_back(transform);
	childs.insert(childs.end(), transform->GetChilds().begin(), transform->GetChilds().end());

	for (int i = 0; i < childs.size(); ++i)
	{
		childs.insert(childs.end(), childs[i]->GetChilds().begin(), childs[i]->GetChilds().end());
	}

	for (int i = 0; i < childs.size(); ++i)
	{
		for (int j = 0; j < _bones.size(); ++j) {
			if (instancedTransforms[j] != nullptr || childs[i]->GetGameObject()->GetName() != _bones[j].name)
				continue;

			instancedTransforms[j] = childs[i];

			int depthLevel = instancedTransforms[j]->GetDepthLevel();
			if (depthOffset == -1) depthOffset = depthLevel;
			while (depthSortedTransformIndices.size() <= depthLevel - depthOffset)
				depthSortedTransformIndices.push_back({});
			depthSortedTransformIndices[depthLevel - depthOffset].push_back(j);

			UpdateBoneTransform(j);
			break;
		}
	}
}

void Skeleton::CreateBuffer()
{
	// Create Buffer
	if (_boneTransformUploadBuffer != nullptr)
		_boneTransformUploadBuffer.release();
	_boneTransformUploadBuffer = make_unique<UploadBuffer<XMFLOAT4X4>>(_bones.size(), false);

	// Create SRV
	_boneTransformSrvIndex = GRAPHIC->GetAndIncreaseSRVHeapIndex();
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(GRAPHIC->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart());
	hDescriptor.Offset(_boneTransformSrvIndex, GRAPHIC->GetCBVSRVDescriptorSize());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = _bones.size();
	srvDesc.Buffer.StructureByteStride = sizeof(XMFLOAT4X4);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	GRAPHIC->GetDevice()->CreateShaderResourceView(_boneTransformUploadBuffer->GetResource(), &srvDesc, hDescriptor);
}
