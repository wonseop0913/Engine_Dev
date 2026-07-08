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

#ifdef BULB_EDITOR
	virtual bool ShowComponentEditorGUI() override;
#endif

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;
	void SaveXML(Bulb::XMLElement compElem) override;

	shared_ptr<Component> Duplicate() override;

	ComponentSnapshot CaptureSnapshot() override;
	void RestoreSnapshot(ComponentSnapshot snapshot) override;

	UINT GetMeshInstanceIndexOffset() { return _meshInstanceIdxOffset; }

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
};

