#include "pch.h"
#include "MeshRenderer.h"

// REGISTER_COMPONENT(MeshRenderer);

MeshRenderer::MeshRenderer() : Super(ComponentType::MeshRenderer)
{

}

MeshRenderer::MeshRenderer(ComponentType type) : Super(type)
{

}

MeshRenderer::~MeshRenderer()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - MeshRenderer:" << _id << "\n";
#endif
}

void MeshRenderer::Render(ID3D12GraphicsCommandList* cmdList, UINT renderState)
{
	if (_mesh == nullptr) return;

	switch (renderState) {
	case RENDERSTATE_MAIN:
		if (RENDER->CheckMeshRender(_mesh)) return;
		RENDER->SetMeshRenderCheckValue(_mesh);
		break;

	case RENDERSTATE_SHADOWMAP:
		if (RENDER->CheckMeshShadowRender(_mesh)) return;
		RENDER->SetMeshShadowRenderCheckValue(_mesh);
		break;
	}

	cmdList->IASetVertexBuffers(0, 1, &_mesh->vertexBufferView);
	cmdList->IASetIndexBuffer(&_mesh->indexBufferView);
	cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT startIndex = RENDER->GetMeshInstanceStartIndex(_mesh);
	cmdList->SetGraphicsRoot32BitConstant(ROOT_PARAM_MESHINFO_C, startIndex, 0);

	cmdList->DrawIndexedInstanced(_mesh->GetIndexCount(), _mesh->GetInstanceCount(), 0, 0, 0);
}

bool MeshRenderer::ShowComponentEditorGUI()
{
	bool deleteFlag = true;

	if (ImGui::CollapsingHeader("MeshRenderer", &deleteFlag, ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::SeparatorText("Mesh");
		{
			if (ImGui::Button("Set Mesh"))
				ImGui::OpenPopup("mesh_select_popup");

			if (ImGui::BeginPopup("mesh_select_popup")) {
				auto meshes = RESOURCE->GetByType<Mesh>();
				for (auto& mesh : meshes) {
					if (ImGui::Selectable(Utils::ToChar(mesh.first))) {
						SetMesh(static_pointer_cast<Mesh>(mesh.second));
					}
				}

				ImGui::EndPopup();
			}

			ImGui::Text(_mesh != nullptr ? Utils::ToChar(_mesh->GetNameW()) : "null");
		}

		ImGui::SeparatorText("Material");
		{
			if (ImGui::Button("Set Material"))
				ImGui::OpenPopup("material_select_popup");

			if (ImGui::BeginPopup("material_select_popup")) {
				auto mats = RESOURCE->GetByType<Material>();
				for (auto& mat : mats) {
					if (ImGui::Selectable(Utils::ToChar(mat.first))) {
						SetMaterial(static_pointer_cast<Material>(mat.second));
					}
				}

				ImGui::EndPopup();
			}

			
			ImGui::Text(_material != nullptr ? Utils::ToChar(_material->GetNameW()) : "null");
		}
	}

	return !deleteFlag;
}

void MeshRenderer::OnDestroy()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "OnDestroy - MeshRenderer:" << _id << "\n";
#endif

	if (_mesh != nullptr) {
		if (!RENDER->IsDestructorRunning()) _mesh->DecreaseInstanceCount();
		_mesh.reset();
	}
	if (_material != nullptr)
		_material.reset();
}

void MeshRenderer::LoadXML(Bulb::XMLElement compElem)
{
	const char* meshPath = compElem.Attribute("Mesh");
	if (meshPath != 0) SetMesh(RESOURCE->LoadMesh(meshPath));

	const char* materialName = compElem.Attribute("Material");
	if (materialName != 0) SetMaterial(RESOURCE->Get<Material>(Utils::ToWString(materialName)));
}

void MeshRenderer::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "MeshRenderer");
	if (_mesh != nullptr) compElem.SetAttribute("Mesh", _mesh->GetPath().c_str());
	if (_material != nullptr) compElem.SetAttribute("Material", _material->GetPath().c_str());
}

shared_ptr<Component> MeshRenderer::Duplicate()
{
	shared_ptr<MeshRenderer> comp = static_pointer_cast<MeshRenderer>(ComponentFactory::Create("MeshRenderer"));

	comp->SetMesh(_mesh);
	comp->SetMaterial(_material);

	return comp;
}

ComponentSnapshot MeshRenderer::CaptureSnapshot()
{
	ComponentSnapshot snapshot;

	snapshot.id = _id;
	snapshot.componentType = "MeshRenderer";

	return snapshot;
}

void MeshRenderer::RestoreSnapshot(ComponentSnapshot snapshot)
{

}

void MeshRenderer::SetMesh(shared_ptr<Mesh> mesh)
{
	if (mesh == nullptr)
		return;

	if (_mesh != nullptr) _mesh->DecreaseInstanceCount();

	_mesh = mesh;
	_mesh->IncreaseInstanceCount();
	if (_mesh->GetMaterial() != nullptr) _material = _mesh->GetMaterial();
	else if (_material == nullptr) _material = RESOURCE->Get<Material>(L"Mat_Default");

	// asset parser 임시조치
	if (GRAPHIC->GetDevice() == nullptr)
		return;
	_mesh->CreateBuffer();

	GetGameObject()->SetFramesDirty();
}
