#pragma once
#include "Resource.h"

#pragma region 전방선언
struct Vertex;
#pragma endregion

class BULB_API Mesh : public Resource
{
	using Super = Resource;
public:
	Mesh(shared_ptr<Geometry> geometry);
	virtual ~Mesh();

public:
	UINT GetIndexCount() { return _geometry->GetIndexCount(); }

	vector<Vertex>& GetVertices() { return _geometry->GetVertices(); }
	void* GetVertexData() { return _geometry->GetVertices().data(); }
	UINT GetVertexCount() { return _geometry->GetVertices().size(); }

	vector<UINT32>& GetIndices() { return _geometry->GetIndices(); }

	void SetMaterial(shared_ptr<Material> mat) {
		_material = mat;
	}
	const shared_ptr<Material> GetMaterial() { return _material; }

	void SetWeights(UINT boneId, vector<BoneWeight>& weights);

	UINT GetInstanceCount() { return _instanceCount; }
	void IncreaseInstanceCount();
	void DecreaseInstanceCount();

	void CreateBuffer();

public:
	ComPtr<ID3D12Resource> vertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> indexBufferGPU = nullptr;

	ComPtr<ID3D12Resource> vertexBufferUploader = nullptr;
	ComPtr<ID3D12Resource> indexBufferUploader = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

private:
	static UINT _totalMeshCount;

	shared_ptr<Geometry> _geometry;
	shared_ptr<Material> _material;

	UINT _instanceCount = 0;
};