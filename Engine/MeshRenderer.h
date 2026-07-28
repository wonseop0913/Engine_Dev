#pragma once
#include "Component.h"
#include "UploadBuffer.h"

class BULB_API MeshRenderer : public Component
{
	using Super = Component;
public:
	MeshRenderer();
	virtual ~MeshRenderer();

	void Render(ID3D12GraphicsCommandList* cmdList, UINT renderState) override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;
	void SaveXML(Bulb::XMLElement compElem) override;

	shared_ptr<Component> Duplicate() override;

	ComponentSnapshot CaptureSnapshot() override;
	void RestoreSnapshot(ComponentSnapshot snapshot) override;

#ifdef BULB_EDITOR
	bool ShowComponentEditorGUI() override;
#endif

	UINT GetMeshInstanceIndexOffset() { return _meshInstanceIdxOffset; }
	void SetMeshInstanceIndexOffset(UINT value) { _meshInstanceIdxOffset = value; }

protected:
	MeshRenderer(ComponentType type);

	shared_ptr<Mesh> _mesh;
	shared_ptr<Material> _material;

	UINT _meshInstanceIdxOffset;

public:
	shared_ptr<Mesh> GetMesh() { return _mesh; }
	void SetMesh(shared_ptr<Mesh> mesh);

	shared_ptr<Material> GetMaterial() { return _material; }
	void SetMaterial(shared_ptr<Material> mat) { 
		_material = mat;
		GetGameObject()->SetFramesDirty();
	}

	// 메시 설정 후 렌더링 관련 정보들 갱신을 하지 않는 경우 사용
	// Asset Parser와 같은 특수한 경우에만 사용
	void SetMeshPlain(shared_ptr<Mesh> mesh);

	void SetMaterialPlain(shared_ptr<Material> mat) { _material = mat; }
};

