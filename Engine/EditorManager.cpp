#include "pch.h"
#include "EditorManager.h"

EditorManager* EditorManager::s_instance = nullptr;

EditorManager::~EditorManager()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - EditorManager\n";
#endif

	_editorCamera.reset();
}

EditorManager* EditorManager::GetInstance()
{
	if (s_instance == nullptr)
		s_instance = new EditorManager();
	return s_instance;
}

Bulb::ProcessResult EditorManager::Delete()
{
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
		return Bulb::ProcessResult::SUCCESS;
	}
	return Bulb::ProcessResult::FAILED_INSTANCE_NOT_FOUND;
}

void EditorManager::Init()
{
	filesystem::path editorSettingPath("./EditorSetting.ini");
	if (!filesystem::exists(editorSettingPath))
		ofstream out("EditorSetting.ini");

	string noneMouseMode = FILEIO->ReadINI("InputSettings", "NoneMouseMode", editorSettingPath.string());
	isNoneMouseMode = noneMouseMode == "true" ? true : false;

	shared_ptr<GameObject> cameraObj = GameObject::Instantiate();
	cameraObj->SetName("EditorCamera");
	cameraObj->SetTag("EditorCamera");
	cameraObj->GetTransform()->SetPosition({ 0, 3, 0 });
	cameraObj->AddComponent(ComponentFactory::Create("Camera"));
	_editorCamera = static_pointer_cast<EditorCamera>(ComponentFactory::Create("EditorCamera"));
	cameraObj->AddComponent(_editorCamera);
	cameraObj->Init();

	LoadMeshes();
	LoadPrefabs();

	_rtvHeapIndex = GRAPHIC->GetAndIncreaseRTVIndex();
	_srvHeapIndex = GRAPHIC->GetAndIncreaseSRVHeapIndex();

	BuildResource();
	BuildSRV();

	_isInitialized = true;
}

void EditorManager::Render(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->ClearRenderTargetView(_rtvHandle, Colors::Black, 0, nullptr);

	shared_ptr<GameObject> seletedObject = EDITORGUI->GetSelectedGameObject();
	if (seletedObject != nullptr) {
		shared_ptr<MeshRenderer> meshRenderer = seletedObject->GetComponent<MeshRenderer>();
		if (meshRenderer == nullptr) return;

		cmdList->OMSetRenderTargets(1, &_rtvHandle, true, nullptr);

		// Terrain의 경우 일단 배제
		if (seletedObject->GetPSOName() == PSO_OPAQUE_SOLID ||
			seletedObject->GetPSOName() == PSO_TRANS_SOLID)
			cmdList->SetPipelineState(RENDER->GetPSO(PSO_OUTLINE_SOLID).Get());

		else if (seletedObject->GetPSOName() == PSO_OPAQUE_SKINNED ||
			seletedObject->GetPSOName() == PSO_TRANS_SKINNED)
			cmdList->SetPipelineState(RENDER->GetPSO(PSO_OUTLINE_SKINNED).Get());

		shared_ptr<Mesh> mesh = meshRenderer->GetMesh();

		if (mesh == nullptr) return;

		cmdList->IASetVertexBuffers(0, 1, &mesh->vertexBufferView);
		cmdList->IASetIndexBuffer(&mesh->indexBufferView);
		cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT startIndex = RENDER->GetMeshInstanceStartIndex(mesh);
		cmdList->SetGraphicsRoot32BitConstant(ROOT_PARAM_MESHINFO_C, startIndex, 0);
		cmdList->SetGraphicsRoot32BitConstant(ROOT_PARAM_MESHINFO_C, meshRenderer->GetMeshInstanceIndexOffset(), 1);

		cmdList->DrawIndexedInstanced(mesh->GetIndexCount(), 1, 0, 0, 0);
	}
}

void EditorManager::Play()
{
	if (Camera::GetCurrentCamera() == nullptr) {
		DEBUG->ErrorLog("Main camera does not exists! Can not start this scene!");
		return;
	}

	auto objects = RENDER->GetObjects();
	
	for (auto& go : objects) {
		GameObjectSnapshot goSnapshot = go->CaptureSnapshot();

		auto comps = go->GetAllComponents();
		for (auto& compVec : comps) {
			for (auto& c : compVec) {
				goSnapshot.compSnapshotIndices.push_back(_compSnapshots.size());
				_compSnapshots.push_back(c->CaptureSnapshot());
			}
		}

		go->SetFramesDirty();

		_objectSnapshots.push_back(goSnapshot);
	}

	_isOnPlay = true;
	SetEditorWindowText(_currentWindowText);
}

void EditorManager::Stop()
{
	SOUND->StopAllSounds();

	auto& objects = RENDER->GetObjects();

	// 런타임중 생성된 오브젝트 삭제
	for (int i = 0; i < objects.size(); ++i) {
		if (!objects[i]->IsSnapshotCaptured()) {
			RENDER->DeleteGameobject(objects[i]);
			--i;
		}
	}

	// 삭제되지 않은 오브젝트들 복구
	for (auto& go : objects) {
		for (int i = 0; i < _objectSnapshots.size(); ++i) {
			GameObjectSnapshot objectSnapshot = _objectSnapshots[i];

			if (go->GetId() == objectSnapshot.id) {
				go->RestoreSnapshot(objectSnapshot);

				RestoreObjectComponents(go, objectSnapshot);

				_objectSnapshots.erase(_objectSnapshots.begin() + i);
				--i;
			}
		}
	}

	// 삭제된 오브젝트들 복구
	for (auto& objectSnapshot : _objectSnapshots) {
		shared_ptr<GameObject> go = GameObject::Instantiate();
		go->RestoreSnapshot(objectSnapshot);

		RestoreObjectComponents(go, objectSnapshot);
	}

	_objectSnapshots.clear();
	_compSnapshots.clear();

	_isOnPlay = false;
	SetEditorWindowText(_currentWindowText);
}

void EditorManager::SetEditorWindowText(string text)
{
	_currentWindowText = text;
	SetWindowText(GRAPHIC->GetMainWnd(), Utils::ToWString("Bulb Engine | " + _currentWindowText + (_isOnPlay ? " - On Play" : "")).c_str());
}

void EditorManager::LoadMeshes()
{
	filesystem::path p = RESOURCE_PATH_MESH;
	filesystem::recursive_directory_iterator iter(p);

	for (auto& i = iter; i != filesystem::end(iter); i++)
	{
		if (filesystem::is_directory(i->path())) continue;

		string pathStr = i->path().string();
		shared_ptr<Mesh> mesh = RESOURCE->LoadMesh(pathStr);
	}
}

void EditorManager::LoadPrefabs()
{
	filesystem::path p = RESOURCE_PATH_PREFAB;
	filesystem::recursive_directory_iterator iter(p);

	for (auto& i = iter; i != filesystem::end(iter); i++)
	{
		if (filesystem::is_directory(i->path())) continue;

		_prefabDirectories.push_back(i->path().string());
	}
}

void EditorManager::BuildSRV()
{
}

void EditorManager::BuildResource()
{
	// RTV
	D3D12_RESOURCE_DESC texDesc =
		CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_R8_UNORM, 
			GRAPHIC->GetAppDesc().clientWidth, 
			GRAPHIC->GetAppDesc().clientHeight
		);
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_R8_UNORM;
	clearValue.Color[0] = 0.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 1.0f;

	ThrowIfFailed(GRAPHIC->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearValue,
		IID_PPV_ARGS(&_outlineRenderTarget)));

	_rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		GRAPHIC->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart(),
		_rtvHeapIndex,
		GRAPHIC->GetRTVDescriptorSize()
	);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	GRAPHIC->GetDevice()->CreateRenderTargetView(_outlineRenderTarget.Get(), &rtvDesc, _rtvHandle);

	// SRV
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv(GRAPHIC->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart());
	hCpuSrv.Offset(_srvHeapIndex, GRAPHIC->GetCBVSRVDescriptorSize());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	GRAPHIC->GetDevice()->CreateShaderResourceView(_outlineRenderTarget.Get(), &srvDesc, hCpuSrv);

	_srvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		GRAPHIC->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart(),
		_srvHeapIndex,
		GRAPHIC->GetCBVSRVDescriptorSize());
}

void EditorManager::RestoreObjectComponents(shared_ptr<GameObject> go, GameObjectSnapshot objectSnapshot)
{
	auto comps = go->GetAllComponents();
	for (int compIdx : objectSnapshot.compSnapshotIndices) {
		ComponentSnapshot compSnapshot = _compSnapshots[compIdx];

		bool flag = false;
		for (auto& compVec : comps) {
			for (auto& c : compVec) {
				if (c->GetID() == compSnapshot.id) {
					c->RestoreSnapshot(compSnapshot);
					flag = true;
					break;
				}
			}
			if (flag) break;
		}
		// 런타임 중에 삭제된 경우
		if (!flag) {
			shared_ptr<Component> comp = ComponentFactory::Create(compSnapshot.componentType);
			go->AddComponent(comp);
			comp->RestoreSnapshot(compSnapshot);
		}
	}
}
