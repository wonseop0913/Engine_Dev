#include "pch.h"
#include "SkinnedMeshRenderer.h"

// REGISTER_COMPONENT(SkinnedMeshRenderer);

SkinnedMeshRenderer::SkinnedMeshRenderer() : Super(ComponentType::SkinnedMeshRenderer)
{

}

SkinnedMeshRenderer::~SkinnedMeshRenderer()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - SkinnedMeshRenderer:" << _id << "\n";
#endif
}

void SkinnedMeshRenderer::Init()
{
	// 본 데이터가 있는 경우 셰이더 코드의 Structured Buffer
	if (_rootBone == nullptr) {
		SetRootBone(GetTransform()->GetParent()->GetChild(_rootBoneName));
	}

	_skeleton = SKELETON->GetSkeleton(_rootBone);
	if (_skeleton == nullptr)
		DEBUG->ErrorLog("No Skeleton Exists!");

	// PSO가 알파블렌딩을 사용하도록 명시된 경우만 알파블렌딩 사용 PSO로 변경, 그 이외에는 기본
	string pso = GetGameObject()->GetPSOName();
	if (pso != PSO_TRANS_SKINNED && pso != PSO_TRANS_SOLID)
		GetGameObject()->SetPSOName(PSO_OPAQUE_SKINNED);
	else
		GetGameObject()->SetPSOName(PSO_TRANS_SKINNED);
}

// 스킨드 메쉬에 대한 인스턴싱은 이뤄지지 않고있음.
void SkinnedMeshRenderer::Render(ID3D12GraphicsCommandList* cmdList, UINT renderState)
{
	if (_rootBone == nullptr) return;

	CD3DX12_GPU_DESCRIPTOR_HANDLE bone(GRAPHIC->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart());
	bone.Offset(_skeleton->GetBoneTransformSRVIndex(), GRAPHIC->GetCBVSRVDescriptorSize());

	cmdList->SetGraphicsRootDescriptorTable(ROOT_PARAM_BONE_SB, bone);

	cmdList->IASetVertexBuffers(0, 1, &_mesh->vertexBufferView);
	cmdList->IASetIndexBuffer(&_mesh->indexBufferView);
	cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT startIndex = RENDER->GetMeshInstanceStartIndex(_mesh);
	cmdList->SetGraphicsRoot32BitConstant(ROOT_PARAM_MESHINFO_C, startIndex, 0);
	cmdList->SetGraphicsRoot32BitConstant(ROOT_PARAM_MESHINFO_C, -1, 1);
	// cmdList->SetGraphicsRoot32BitConstant(ROOT_PARAM_MESHINFO_C, _meshInstanceIdxOffset, 1);

	cmdList->DrawIndexedInstanced(_mesh->GetIndexCount(), 1, 0, 0, 0);
}

bool SkinnedMeshRenderer::ShowComponentEditorGUI()
{
	if (ImGui::CollapsingHeader("SkinnedMeshRenderer", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::SeparatorText("Mesh");
		ImGui::Text(Utils::ToChar(_mesh->GetNameW()));

		ImGui::SeparatorText("Material");
		ImGui::Text(Utils::ToChar(_material->GetNameW()));

		ImGui::SeparatorText("RootBone");
		ImGui::Text(_rootBone->GetGameObject()->GetName().c_str());
	}

	return false;
}

void SkinnedMeshRenderer::OnDestroy()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "OnDestroy - SkinnedMeshRenderer:" << _id << "\n";
#endif

	if (_rootBone != nullptr)
		_rootBone.reset();
	if (_skeleton != nullptr)
		_skeleton.reset();
	if (_mesh != nullptr)
		_mesh.reset();
	if (_material != nullptr)
		_material.reset();
}

void SkinnedMeshRenderer::LoadXML(Bulb::XMLElement compElem)
{
	const char* meshPath = compElem.Attribute("Mesh");
	if (meshPath != 0) SetMesh(RESOURCE->LoadMesh(meshPath));

	const char* materialName = compElem.Attribute("Material");
	if (materialName != 0) SetMaterial(RESOURCE->Get<Material>(Utils::ToWString(materialName)));

	const char* rootBoneName = compElem.Attribute("RootBoneName");
	if (rootBoneName != 0) _rootBoneName = rootBoneName;
}

void SkinnedMeshRenderer::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "SkinnedMeshRenderer");

	if (_mesh != nullptr) compElem.SetAttribute("Mesh", _mesh->GetPath().c_str());
	if (_material != nullptr) compElem.SetAttribute("Material", _material->GetPath().c_str());
	compElem.SetAttribute("RootBoneName", _rootBoneName.c_str());
}

shared_ptr<Component> SkinnedMeshRenderer::Duplicate()
{
	shared_ptr<SkinnedMeshRenderer> comp = static_pointer_cast<SkinnedMeshRenderer>(ComponentFactory::Create("SkinnedMeshRenderer"));

	comp->SetMesh(_mesh);
	comp->SetMaterial(_material);
	comp->SetRootBone(_rootBoneName);

	return comp;
}

ComponentSnapshot SkinnedMeshRenderer::CaptureSnapshot()
{
	ComponentSnapshot snapshot;

	snapshot.id = _id;
	snapshot.componentType = "SkinnedMeshRenderer";

	return snapshot;
}

void SkinnedMeshRenderer::RestoreSnapshot(ComponentSnapshot snapshot)
{

}

void SkinnedMeshRenderer::SetRootBone(const shared_ptr<Transform> rootBone)
{
	_rootBone = rootBone;
	_rootBoneName = _rootBone->GetGameObject()->GetName();
}
