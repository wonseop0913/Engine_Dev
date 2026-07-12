#include "pch.h"
#include "RenderManager.h"

RenderManager* RenderManager::s_instance = nullptr;

RenderManager::~RenderManager()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - RenderManager\n";
#endif

	_isDestructorRunning = true;

	_meshInstanceStartIndex.clear();

	// Release Lights
	for (shared_ptr<Light> l : _lights) {
		l.reset();
	}
	_lights.clear();

	// Release Objects
	for (shared_ptr<GameObject> go : _objects) {
		go->OnDestroy();
		go.reset();
	}
	_objects.clear();

	for (vector<shared_ptr<GameObject>>& gos : _objectsSortedPSO) {
		for (shared_ptr<GameObject> go : gos)
			go.reset();
		gos.clear();
	}

	_skyboxObject->OnDestroy();
	_skyboxObject.reset();

	// Release Textures
	_skyboxTexture.reset();
}

RenderManager* RenderManager::GetInstance()
{
	if (s_instance == nullptr)
		s_instance = new RenderManager();
	return s_instance;
}

Bulb::ProcessResult RenderManager::Delete()
{
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
		return Bulb::ProcessResult::SUCCESS;
	}
	return Bulb::ProcessResult::FAILED_INSTANCE_NOT_FOUND;
}

void RenderManager::Init()
{
	for (int i = 0; i < 3; i++) {
		ThrowIfFailed(GRAPHIC->GetDevice()->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&_cmdListAllocs[i])));

		ThrowIfFailed(GRAPHIC->GetDevice()->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			_cmdListAllocs[i],
			nullptr,
			IID_PPV_ARGS(&_cmdLists[i])));

		ThrowIfFailed(_cmdLists[i]->Close());
	}

	BuildFrameResources();
	BuildRootSignature();
	BuildInputLayout();


#ifdef BULB_EDITOR
	ENGINEGUI->Init();
#endif
	RESOURCE->Init();

	BuildPSOs();

	if (_skyboxTexSrvHeapIndex == -1)
		SetSkyboxTexture(RESOURCE->Get<Texture>(L"Tex_DefaultSkybox"));
	
	_shadowMap = make_unique<ShadowMap>(2048, 2048);
	_shadowMap->BuildDescriptors();

	_skyboxObject = make_shared<GameObject>();
	shared_ptr<MeshRenderer> skyboxRenderer = make_shared<MeshRenderer>();
	_skyboxObject->AddComponent(skyboxRenderer);
	skyboxRenderer->SetMesh(RESOURCE->Get<Mesh>(DEFAULT_MESH_SKYBOX));

	for (auto& o : _objects)
		o->Init();
}

void RenderManager::PreUpdate()
{
	if (_iblBRDFlutTexture == nullptr) {

		_iblBRDFlutTexture = RESOURCE->Get<Texture>(L"..\\Resources\\Textures\\EngineDefault\\ibl_brdf_lut.png");
		_iblBRDFlutTextureIdx = _iblBRDFlutTexture->GetSRVHeapIndex();
	}

	for (int i = 0; i < _objects.size(); ++i) {
		if (_objects[i] != nullptr)
			_objects[i]->PreUpdate();

		// 런타임 중에 삭제된 경우
		if (_objects[i] == nullptr) {
			_objects.erase(_objects.begin() + i);
			--i;
		}
	}
}

void RenderManager::Update()
{
	// Frame Resource Update
	_currFrameResource = _frameResources[_currFrameResourceIndex].get();

	GRAPHIC->WaitForFence(_currFrameResource->fence);

	// Objects Update
	for (auto& o : _objects) {
		// if (o->GetComponentCount() <= 1) continue;

		o->Update();
	}
	
	for (auto& l : _lights) {
		l->Update();
	}

	_currFrameResource->Update();
}

void RenderManager::Render()
{
	{
		ThrowIfFailed(_currFrameResource->cmdListAlloc[0]->Reset());
		ThrowIfFailed(_cmdLists[0]->Reset(_currFrameResource->cmdListAlloc[0], nullptr));
		ThrowIfFailed(_currFrameResource->cmdListAlloc[1]->Reset());
		ThrowIfFailed(_cmdLists[1]->Reset(_currFrameResource->cmdListAlloc[1], nullptr));
		ThrowIfFailed(_currFrameResource->cmdListAlloc[2]->Reset());
		ThrowIfFailed(_cmdLists[2]->Reset(_currFrameResource->cmdListAlloc[2], nullptr));
	}

	bool isMsaaEnabled = GRAPHIC->IsMSAAEnabled();
	ID3D12Resource* backBuffer = GRAPHIC->GetCurrentBackBuffer();
	ID3D12Resource* mainRenderTarget = GRAPHIC->GetMainRenderTarget();
	ID3D12Resource* renderTarget = isMsaaEnabled ? GRAPHIC->GetMSAARenderTarget() : mainRenderTarget;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = isMsaaEnabled ? GRAPHIC->GetMSAARTVHandle() : GRAPHIC->GetMainRTVHandle();

	// ShadowMap Pass
	_futures[0] = THREAD->EnqueueJob([this] {
		SetStateDefault(_cmdLists[0]);

		_cmdLists[0]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_shadowMap->GetResource(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

		_cmdLists[0]->RSSetViewports(1, &_shadowMap->GetViewport());
		_cmdLists[0]->RSSetScissorRects(1, &_shadowMap->GetScissorRect());

		_cmdLists[0]->ClearDepthStencilView(_shadowMap->GetDsv(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		_cmdLists[0]->OMSetRenderTargets(0, nullptr, false, &_shadowMap->GetDsv());

		// Shadow Map Solid
		_cmdLists[0]->SetPipelineState(_PSOs[PSO_SHADOWMAP].Get());

		for (auto& o : _objectsSortedPSO[PSO_IDX_OPAQUE_SOLID]) {
			o->Render(_cmdLists[0], RENDERSTATE_SHADOWMAP);
		}
		for (auto& o : _objectsSortedPSO[PSO_IDX_TRANS_SOLID]) {
			o->Render(_cmdLists[0], RENDERSTATE_SHADOWMAP);
		}

		// Shadow Map Skinned
		_cmdLists[0]->SetPipelineState(_PSOs[PSO_SHADOWMAP_SKINNED].Get());

		for (auto& o : _objectsSortedPSO[PSO_IDX_OPAQUE_SKINNED]) {
			o->Render(_cmdLists[0], RENDERSTATE_SHADOWMAP);
		}
		for (auto& o : _objectsSortedPSO[PSO_IDX_TRANS_SKINNED]) {
			o->Render(_cmdLists[0], RENDERSTATE_SHADOWMAP);
		}

		RefreshMeshShadowRenderCheckMap();

		SetStateTerrain(_cmdLists[0]);

		// Terrain
		if (_terrains.size() > 0) {
			_cmdLists[0]->SetPipelineState(_PSOs[PSO_SHADOWMAP_TERRAIN].Get());
			for (auto& t : _terrains) {
				t->Render(_cmdLists[0], RENDERSTATE_SHADOWMAP);
			}
		}

		_cmdLists[0]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_shadowMap->GetResource(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));

		ThrowIfFailed(_cmdLists[0]->Close());
	});

	// Main Pass
	_futures[1] = THREAD->EnqueueJob([this, renderTarget, rtvHandle] {
		SetStateDefault(_cmdLists[1]);

		_cmdLists[1]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTarget,
			D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

		_cmdLists[1]->RSSetViewports(1, &GRAPHIC->GetViewport());
		_cmdLists[1]->RSSetScissorRects(1, &GRAPHIC->GetScissorRect());

		_cmdLists[1]->ClearRenderTargetView(rtvHandle, Colors::Black, 0, nullptr);
		_cmdLists[1]->ClearDepthStencilView(GRAPHIC->GetDSVHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		_cmdLists[1]->OMSetRenderTargets(1, &rtvHandle, true, &GRAPHIC->GetDSVHandle());

		// Skybox
		_cmdLists[1]->SetPipelineState(_PSOs[PSO_SKYBOX].Get());
		_skyboxObject->Render(_cmdLists[1], RENDERSTATE_MAIN);

		// Opaque Solid
		if (_objectsSortedPSO[PSO_IDX_OPAQUE_SOLID].size() > 0) {
			_cmdLists[1]->SetPipelineState(_PSOs[PSO_OPAQUE_SOLID].Get());
			for (auto& o : _objectsSortedPSO[PSO_IDX_OPAQUE_SOLID]) {
				o->Render(_cmdLists[1], RENDERSTATE_MAIN);
			}
		}

		// Opaque Skinned
		if (_objectsSortedPSO[PSO_IDX_OPAQUE_SKINNED].size() > 0) {
			_cmdLists[1]->SetPipelineState(_PSOs[PSO_OPAQUE_SKINNED].Get());
			for (auto& o : _objectsSortedPSO[PSO_IDX_OPAQUE_SKINNED]) {
				o->Render(_cmdLists[1], RENDERSTATE_MAIN);
			}
		}

		// Trans Solid
		if (_objectsSortedPSO[PSO_IDX_TRANS_SOLID].size() > 0) {
			_cmdLists[1]->SetPipelineState(_PSOs[PSO_TRANS_SOLID].Get());
			for (auto& o : _objectsSortedPSO[PSO_IDX_TRANS_SOLID]) {
				o->Render(_cmdLists[1], RENDERSTATE_MAIN);
			}
		}

		// Trans Skinned
		if (_objectsSortedPSO[PSO_IDX_TRANS_SKINNED].size() > 0) {
			_cmdLists[1]->SetPipelineState(_PSOs[PSO_TRANS_SKINNED].Get());
			for (auto& o : _objectsSortedPSO[PSO_IDX_TRANS_SKINNED]) {
				o->Render(_cmdLists[1], RENDERSTATE_MAIN);
			}
		}

		RefreshMeshRenderCheckMap();

#ifdef BULB_EDITOR
		_cmdLists[1]->SetPipelineState(_PSOs[PSO_DEBUG_PHYSICS].Get());
		DEBUG->Render(_cmdLists[1]);
#endif

		SetStateTerrain(_cmdLists[1]);

		// Terrain
		if (_terrains.size() > 0) {
			_cmdLists[1]->SetPipelineState(_PSOs[PSO_TERRAIN].Get());
			for (auto& t : _terrains) {
				t->Render(_cmdLists[1], RENDERSTATE_MAIN);
			}
		}

#ifdef BULB_EDITOR
		SetStateDefault(_cmdLists[1]);

		EDITOR->Render(_cmdLists[1]);
#endif

		ThrowIfFailed(_cmdLists[1]->Close());
	});

	// Particle System Update / Render
	{
		SetStateParticle(_cmdLists[2]);

		PARTICLE->Update(_cmdLists[2]);

		_cmdLists[2]->RSSetViewports(1, &GRAPHIC->GetViewport());
		_cmdLists[2]->RSSetScissorRects(1, &GRAPHIC->GetScissorRect());

		_cmdLists[2]->OMSetRenderTargets(1, &rtvHandle, true, &GRAPHIC->GetDSVHandle());

		_cmdLists[2]->SetPipelineState(_PSOs[PSO_PARTICLE_RENDER].Get());
		_cmdLists[2]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		PARTICLE->Render(_cmdLists[2], RENDERSTATE_SUB);

		// MSAA가 꺼진 경우: 그냥 MainRenderTarget을 사용
		// MSAA가 켜진 경우: MSAA->Main으로 복사 후 Main->Back복사
		if (isMsaaEnabled) {
			_cmdLists[2]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTarget,
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE));

			_cmdLists[2]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mainRenderTarget,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RESOLVE_DEST));

			_cmdLists[2]->ResolveSubresource(mainRenderTarget, 0, renderTarget, 0, GRAPHIC->GetBackBufferFormat());

			_cmdLists[2]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mainRenderTarget,
				D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		}

		_cmdLists[2]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffer,
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		_cmdLists[2]->OMSetRenderTargets(1, &GRAPHIC->GetCurrentBackBufferView(), true, nullptr);

		_cmdLists[2]->SetPipelineState(_PSOs["postprocessing"].Get());
		_cmdLists[2]->SetGraphicsRootSignature(_rootSignatureDefault.Get());

		_cmdLists[2]->SetGraphicsRootDescriptorTable(ROOT_PARAM_MAINPASSREF_SR, GRAPHIC->GetMainSRVHandle());
		_cmdLists[2]->SetGraphicsRoot32BitConstant(ROOT_PARAM_RENDERREF_C, GRAPHIC->GetMainRenderTargetSRVIndex(), 0);
		_cmdLists[2]->SetGraphicsRoot32BitConstant(ROOT_PARAM_RENDERREF_C, EDITOR->GetSRVIndex(), 1);

		auto quad = RESOURCE->Get<Mesh>(DEFAULT_MESH_QUAD);
		_cmdLists[2]->IASetVertexBuffers(0, 1, &quad->vertexBufferView);
		_cmdLists[2]->IASetIndexBuffer(&quad->indexBufferView);
		_cmdLists[2]->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		_cmdLists[2]->DrawIndexedInstanced(quad->GetIndexCount(), 1, 0, 0, 0);

		// UI
		SetStateUI(_cmdLists[2]);

		_cmdLists[2]->SetPipelineState(_PSOs[PSO_UI].Get());
		_cmdLists[2]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		UI->Render(_cmdLists[2]);

#ifdef BULB_EDITOR
		ENGINEGUI->Render(_cmdLists[2]);
#endif

		_cmdLists[2]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffer,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		
		ThrowIfFailed(_cmdLists[2]->Close());
	}

	_futures[0].get();
	_futures[1].get();

	ID3D12CommandList* cmdsLists[] = { _cmdLists[0], _cmdLists[1], _cmdLists[2] };

	auto commaneQueue = GRAPHIC->GetCommandQueue();
	commaneQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	ThrowIfFailed(GRAPHIC->GetSwapChain()->Present(0, 0));

	GRAPHIC->RenderEnd(_currFrameResource);

	_currFrameResourceIndex = (_currFrameResourceIndex + 1) % _numFrameResources;
}

void RenderManager::InitializeOnRuntime()
{
#ifdef BULB_EDITOR
	shared_ptr<GameObject> editorCameraObj;
#endif

	for (shared_ptr<GameObject> go : _objects) {
#ifdef BULB_EDITOR
		if (go->GetTag() == "EditorCamera") {
			editorCameraObj = go;
			continue;
		}
#endif
		go->OnDestroy();
		go.reset();
	}
	_objects.clear();

	for (vector<shared_ptr<GameObject>>& gos : _objectsSortedPSO) {
		for (shared_ptr<GameObject> go : gos)
			go.reset();
		gos.clear();
	}

#ifdef BULB_EDITOR
	AddGameObject(editorCameraObj);
#endif
}

shared_ptr<GameObject> RenderManager::GetObject(string objName)
{
	for (auto& go : _objects) {
		if (go->GetName() == objName)
			return go;
	}

	return nullptr;
}

shared_ptr<GameObject> RenderManager::GetObjectWithTag(string tag)
{
	for (auto& go : _objects) {
		if (go->GetTag() == tag)
			return go;
	}

	return nullptr;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC RenderManager::CreatePSODesc(vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout, ID3D12RootSignature* rootSignature, wstring vsName, wstring psName, wstring dsName, wstring hsName, wstring gsName)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = CreatePSODesc(rootSignature, vsName, psName, dsName, hsName, gsName);
	psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };

	return psoDesc;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC RenderManager::CreatePSODesc(ID3D12RootSignature* rootSignature, wstring vsName, wstring psName, wstring dsName, wstring hsName, wstring gsName)
{
	AppDesc appDesc = GRAPHIC->GetAppDesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	psoDesc.pRootSignature = rootSignature;

	if (!vsName.empty())
	{
		auto vertexShader = RESOURCE->Get<Shader>(vsName)->GetBlob();
		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(vertexShader->GetBufferPointer()),
			vertexShader->GetBufferSize()
		};
	}

	if (!psName.empty())
	{
		auto pixelShader = RESOURCE->Get<Shader>(psName)->GetBlob();
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(pixelShader->GetBufferPointer()),
			pixelShader->GetBufferSize()
		};
	}

	if (!dsName.empty())
	{
		auto domainShader = RESOURCE->Get<Shader>(dsName)->GetBlob();
		psoDesc.DS =
		{
			reinterpret_cast<BYTE*>(domainShader->GetBufferPointer()),
			domainShader->GetBufferSize()
		};
	}

	if (!hsName.empty())
	{
		auto hullShader = RESOURCE->Get<Shader>(hsName)->GetBlob();
		psoDesc.HS =
		{
			reinterpret_cast<BYTE*>(hullShader->GetBufferPointer()),
			hullShader->GetBufferSize()
		};
	}

	if (!gsName.empty())
	{
		auto geoShader = RESOURCE->Get<Shader>(gsName)->GetBlob();
		psoDesc.GS =
		{
			reinterpret_cast<BYTE*>(geoShader->GetBufferPointer()),
			geoShader->GetBufferSize()
		};
	}

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = GRAPHIC->GetBackBufferFormat();
	psoDesc.SampleDesc.Count = appDesc._4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = appDesc._4xMsaaState ? (appDesc._4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = GRAPHIC->GetDepthStencilFormat();

	return psoDesc;
}

D3D12_COMPUTE_PIPELINE_STATE_DESC RenderManager::CreateCSPSODesc(ID3D12RootSignature* rootSignature, wstring csName)
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature;
	auto computeShader = RESOURCE->Get<Shader>(csName)->GetBlob();
	psoDesc.CS =
	{
		reinterpret_cast<BYTE*>(computeShader->GetBufferPointer()),
		computeShader->GetBufferSize()
	};
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	return psoDesc;
}

#pragma region Setters

void RenderManager::SetCurrPSO(string name)
{
	_currPSO = _PSOs[name];
	_isPSOFixed = true;
}

void RenderManager::SetDefaultPSO()
{
	_currPSO = _PSOs[PSO_OPAQUE_SOLID];
	_isPSOFixed = false;
}

#pragma endregion

void RenderManager::UpdateObjectPSO(shared_ptr<GameObject> obj, string targetPSO)
{
	int objPsoIdx = Temp_GetPSOIndex(obj->GetPSOName());
	int targetPsoIdx = Temp_GetPSOIndex(targetPSO);
	for (int i = 0; i < _objectsSortedPSO[objPsoIdx].size(); i++)
	{
		if (_objectsSortedPSO[objPsoIdx][i] == obj)
		{
			_objectsSortedPSO[objPsoIdx].erase(_objectsSortedPSO[objPsoIdx].begin() + i);
			_objectsSortedPSO[targetPsoIdx].push_back(obj);
			break;
		}
	}
}

UINT RenderManager::Temp_GetPSOIndex(string name)
{
	if (name == PSO_OPAQUE_SOLID)		return 0;
	if (name == PSO_OPAQUE_SKINNED)		return 1;
	if (name == PSO_TRANS_SOLID)		return 2;
	if (name == PSO_TRANS_SKINNED)		return 3;
	if (name == PSO_SKYBOX)				return 4;
	if (name == PSO_SHADOWMAP)			return 5;
	if (name == PSO_SHADOWMAP_SKINNED)	return 6;
	if (name == PSO_WIREFRAME)			return 7;
	if (name == PSO_DEBUG_PHYSICS)		return 8;
	if (name == PSO_DEBUG_SHADOW)		return 9;
	if (name == PSO_PARTICLE_UPDATE)	return 10;
	if (name == PSO_PARTICLE_RENDER)	return 11;
}

void RenderManager::BuildFrameResources()
{
	for (int i = 0; i < _numFrameResources; ++i)
		_frameResources.push_back(make_unique<FrameResource>());
}

shared_ptr<GameObject> RenderManager::AddGameObject(shared_ptr<GameObject> obj)
{
	for (auto& o : _objects)
	{
		if (obj == o)
			return nullptr;
	}
	obj->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	_objectsSortedPSO[Temp_GetPSOIndex(obj->GetPSOName())].push_back(obj);
	_objects.push_back(move(obj));
	return _objects[_objects.size() - 1];
}


void RenderManager::DeleteGameobject(shared_ptr<GameObject> obj)
{
	vector<shared_ptr<GameObject>> deleteObjs;
	deleteObjs.push_back(obj);

	auto& childs = obj->GetTransform()->GetChilds();
	for (auto& c : childs) {
		deleteObjs.push_back(c->GetGameObject());
	}
	obj->OnDestroy();

	for (auto& o : deleteObjs) {
		for (int i = 0; i < _objects.size(); i++) {
			if (o == _objects[i]) {
				_objects[i].reset();
				// _objects.erase(_objects.begin() + i);
				break;
			}
		}

		int objPsoIdx = Temp_GetPSOIndex(o->GetPSOName());
		for (int i = 0; i < _objectsSortedPSO[objPsoIdx].size(); i++) {
			if (o == _objectsSortedPSO[objPsoIdx][i]) {
				_objectsSortedPSO[objPsoIdx].erase(_objectsSortedPSO[objPsoIdx].begin() + i);
				break;
			}
		}
	}
}

void RenderManager::DeleteLight(shared_ptr<Light> light)
{
	for (int i = 0; i < _lights.size(); ++i) {
		if (_lights[i] == light) {
			_lights.erase(_lights.begin() + i);
		}
	}
}

void RenderManager::DeleteTerrain(shared_ptr<Terrain> terrain)
{
	for (int i = 0; i < _terrains.size(); ++i) {
		if (_terrains[i] == terrain) {
			_terrains.erase(_terrains.begin() + i);
		}
	}
}

void RenderManager::UpdateMeshInstanceStartIndices()
{
	int indexStack = 0;
	auto meshes = RESOURCE->GetByType<Mesh>();
	_meshInstanceStartIndex.clear();
	_meshInstanceStartIndex.resize(meshes.size(), 0);
	for (auto& meshPair : meshes) {
		shared_ptr<Mesh> mesh = static_pointer_cast<Mesh>(meshPair.second);
		_meshInstanceStartIndex[mesh->GetID()] = indexStack;
		indexStack += mesh->GetInstanceCount();
	}

	// temp
	for (auto& go : _objects) {
		if (go != nullptr) go->SetFramesDirty();
	}
}

// �̻�� �޽� ������ Ȯ�ο� �޼ҵ�
void RenderManager::RefreshMeshRenderCheckMap()
{
	int meshCount = _meshRenderCheckMap.size();
	_meshRenderCheckMap.clear();
	_meshRenderCheckMap.resize(meshCount, false);
}

void RenderManager::RefreshMeshShadowRenderCheckMap()
{
	int meshCount = _meshShadowRenderCheckMap.size();
	_meshShadowRenderCheckMap.clear();
	_meshShadowRenderCheckMap.resize(meshCount, false);
}

#pragma region Build_Render_Components

void RenderManager::BuildPSO(string name, D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc)
{
	ThrowIfFailed(GRAPHIC->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(_PSOs[name].GetAddressOf())));
}

void RenderManager::BuildPSO(string name, D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc)
{
	ThrowIfFailed(GRAPHIC->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(_PSOs[name].GetAddressOf())));
}

void RenderManager::BuildRootSignature()
{
	// Common
	CD3DX12_DESCRIPTOR_RANGE matTable;
	matTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, REGISTER_NUM_MAT_SB, 0);
	CD3DX12_DESCRIPTOR_RANGE lightTable;
	lightTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, REGISTER_NUM_LIGHT_SB, 0);

	// Textures
	CD3DX12_DESCRIPTOR_RANGE cubemapTable;
	cubemapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, REGISTER_NUM_SKYBOX_SR, 0);
	CD3DX12_DESCRIPTOR_RANGE shadowTexTable;
	shadowTexTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, REGISTER_NUM_SHADOWMAP_SR, 0);
	CD3DX12_DESCRIPTOR_RANGE mainPassRenderRefTable;
	mainPassRenderRefTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, REGISTER_NUM_MAINPASSREF_SR, 0);
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, DEFAULT_TEXTURE_ARR_SIZE, 5, 0);

	/* Space 1 */
	// Default
	CD3DX12_DESCRIPTOR_RANGE instanceTable;
	instanceTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, REGISTER_NUM_INSTANCE_SB, 1);
	CD3DX12_DESCRIPTOR_RANGE boneTable;
	boneTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, REGISTER_NUM_BONE_SB, 1);

	// Terrain
	CD3DX12_DESCRIPTOR_RANGE terrainTable;
	terrainTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);

	// UI
	CD3DX12_DESCRIPTOR_RANGE uiTable;
	uiTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);

	auto staticSamplers = GetStaticSamplers();

	// Default
	{
		CD3DX12_ROOT_PARAMETER slotRootParameter[ROOT_PARAMETER_COUNT_DEFAULT];

		slotRootParameter[ROOT_PARAM_MATERIAL_SB].InitAsDescriptorTable(1, &matTable);
		slotRootParameter[ROOT_PARAM_LIGHT_SB].InitAsDescriptorTable(1, &lightTable);
		slotRootParameter[ROOT_PARAM_SKYBOX_SR].InitAsDescriptorTable(1, &cubemapTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[ROOT_PARAM_SHADOWMAP_SR].InitAsDescriptorTable(1, &shadowTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[ROOT_PARAM_MAINPASSREF_SR].InitAsDescriptorTable(1, &mainPassRenderRefTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[ROOT_PARAM_TEXTURE_ARR].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[ROOT_PARAM_CLIENTINFO_C].InitAsConstants(2, REGISTER_NUM_CLIENTINFO_C);
		slotRootParameter[ROOT_PARAM_LIGHTINFO_C].InitAsConstants(2, REGISTER_NUM_LIGHTINFO_C);
		slotRootParameter[ROOT_PARAM_CAMERA_CB].InitAsConstantBufferView(REGISTER_NUM_CAMERA_CB);
		slotRootParameter[ROOT_PARAM_MESHINFO_C].InitAsConstants(2, REGISTER_NUM_MESHINFO_C);
		slotRootParameter[ROOT_PARAM_RENDERREF_C].InitAsConstants(2, REGISTER_NUM_RENDERREF_C);

		slotRootParameter[ROOT_PARAM_INSTCANCE_SB].InitAsDescriptorTable(1, &instanceTable);
		slotRootParameter[ROOT_PARAM_BONE_SB].InitAsDescriptorTable(1, &boneTable);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(ROOT_PARAMETER_COUNT_DEFAULT, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(GRAPHIC->GetDevice()->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(_rootSignatureDefault.GetAddressOf())));
	}

	// Terrain
	{
		CD3DX12_ROOT_PARAMETER slotRootParameter[ROOT_PARAMETER_COUNT_TERRAIN];

		slotRootParameter[ROOT_PARAM_MATERIAL_SB].InitAsDescriptorTable(1, &matTable);
		slotRootParameter[ROOT_PARAM_LIGHT_SB].InitAsDescriptorTable(1, &lightTable);
		slotRootParameter[ROOT_PARAM_SKYBOX_SR].InitAsDescriptorTable(1, &cubemapTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[ROOT_PARAM_SHADOWMAP_SR].InitAsDescriptorTable(1, &shadowTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[ROOT_PARAM_MAINPASSREF_SR].InitAsDescriptorTable(1, &mainPassRenderRefTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[ROOT_PARAM_TEXTURE_ARR].InitAsDescriptorTable(1, &texTable);
		slotRootParameter[ROOT_PARAM_CLIENTINFO_C].InitAsConstants(2, REGISTER_NUM_CLIENTINFO_C);
		slotRootParameter[ROOT_PARAM_LIGHTINFO_C].InitAsConstants(2, REGISTER_NUM_LIGHTINFO_C);
		slotRootParameter[ROOT_PARAM_CAMERA_CB].InitAsConstantBufferView(REGISTER_NUM_CAMERA_CB);
		slotRootParameter[ROOT_PARAM_MESHINFO_C].InitAsConstants(2, REGISTER_NUM_MESHINFO_C);
		slotRootParameter[ROOT_PARAM_RENDERREF_C].InitAsConstants(2, REGISTER_NUM_RENDERREF_C);

		slotRootParameter[ROOT_PARAM_TERRAININFO_C].InitAsConstants(4, REGISTER_NUM_TERRAININFO_C, 1);
		slotRootParameter[ROOT_PARAM_TERRAIN_SB].InitAsDescriptorTable(1, &terrainTable);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(ROOT_PARAMETER_COUNT_TERRAIN, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(GRAPHIC->GetDevice()->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(_rootSignatureTerrain.GetAddressOf())));
	}

	// Particle
	{
		CD3DX12_ROOT_PARAMETER slotRootParameter[ROOT_PARAMETER_COUNT_PARTICLE];

		slotRootParameter[ROOT_PARAM_MATERIAL_SB].InitAsDescriptorTable(1, &matTable);
		slotRootParameter[ROOT_PARAM_LIGHT_SB].InitAsDescriptorTable(1, &lightTable);
		slotRootParameter[ROOT_PARAM_SKYBOX_SR].InitAsDescriptorTable(1, &cubemapTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[ROOT_PARAM_SHADOWMAP_SR].InitAsDescriptorTable(1, &shadowTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[ROOT_PARAM_MAINPASSREF_SR].InitAsDescriptorTable(1, &mainPassRenderRefTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[ROOT_PARAM_TEXTURE_ARR].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[ROOT_PARAM_CLIENTINFO_C].InitAsConstants(2, REGISTER_NUM_CLIENTINFO_C);
		slotRootParameter[ROOT_PARAM_LIGHTINFO_C].InitAsConstants(2, REGISTER_NUM_LIGHTINFO_C);
		slotRootParameter[ROOT_PARAM_CAMERA_CB].InitAsConstantBufferView(REGISTER_NUM_CAMERA_CB);
		slotRootParameter[ROOT_PARAM_MESHINFO_C].InitAsConstants(2, REGISTER_NUM_MESHINFO_C);
		slotRootParameter[ROOT_PARAM_RENDERREF_C].InitAsConstants(2, REGISTER_NUM_RENDERREF_C);

		slotRootParameter[ROOT_PARAM_PARTICLES_RW].InitAsUnorderedAccessView(REGISTER_NUM_PARTICLES_RW, 1);
		slotRootParameter[ROOT_PARAM_EMITTER_CB].InitAsConstants(sizeof(EmitterSetting) / 4, REGISTER_NUM_EMITTER_CB, 1);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(ROOT_PARAMETER_COUNT_PARTICLE, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(GRAPHIC->GetDevice()->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(_rootSignatureParticle.GetAddressOf())));
	}

	// UI
	{
		CD3DX12_ROOT_PARAMETER slotRootParameter[ROOT_PARAMETER_COUNT_UI];

		slotRootParameter[ROOT_PARAM_MATERIAL_SB].InitAsDescriptorTable(1, &matTable);
		slotRootParameter[ROOT_PARAM_LIGHT_SB].InitAsDescriptorTable(1, &lightTable);
		slotRootParameter[ROOT_PARAM_SKYBOX_SR].InitAsDescriptorTable(1, &cubemapTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[ROOT_PARAM_SHADOWMAP_SR].InitAsDescriptorTable(1, &shadowTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[ROOT_PARAM_MAINPASSREF_SR].InitAsDescriptorTable(1, &mainPassRenderRefTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[ROOT_PARAM_TEXTURE_ARR].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[ROOT_PARAM_CLIENTINFO_C].InitAsConstants(2, REGISTER_NUM_CLIENTINFO_C);
		slotRootParameter[ROOT_PARAM_LIGHTINFO_C].InitAsConstants(2, REGISTER_NUM_LIGHTINFO_C);
		slotRootParameter[ROOT_PARAM_CAMERA_CB].InitAsConstantBufferView(REGISTER_NUM_CAMERA_CB);
		slotRootParameter[ROOT_PARAM_MESHINFO_C].InitAsConstants(2, REGISTER_NUM_MESHINFO_C);
		slotRootParameter[ROOT_PARAM_RENDERREF_C].InitAsConstants(2, REGISTER_NUM_RENDERREF_C);

		slotRootParameter[ROOT_PARAM_UI_SB].InitAsDescriptorTable(1, &uiTable);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(ROOT_PARAMETER_COUNT_UI, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(GRAPHIC->GetDevice()->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(_rootSignatureUI.GetAddressOf())));
	}
}

void RenderManager::BuildInputLayout()
{
	_solidInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	_skinnedInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, 60, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	_colliderDebugInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void RenderManager::BuildPSOs()
{
	// Default Layout
	auto opaqueSolid = CreatePSODesc(_solidInputLayout, _rootSignatureDefault.Get(), SHADER_VERTEX_DEFAULT, SHADER_PIXEL_DEFAULT);
	auto opaqueSkinned = CreatePSODesc(_skinnedInputLayout, _rootSignatureDefault.Get(), SHADER_VERTEX_SKINNED, SHADER_PIXEL_DEFAULT);

	auto transSolid = opaqueSolid;
	auto transSkinned = opaqueSkinned;

	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	transSolid.BlendState = blendDesc;
	transSolid.DepthStencilState.DepthEnable = TRUE;
	transSolid.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	transSkinned.BlendState = blendDesc;
	transSkinned.DepthStencilState.DepthEnable = TRUE;
	transSkinned.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	// Wireframe Layout
	auto opaqueWireframe = opaqueSkinned;
	opaqueWireframe.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;

	// Skybow Layout
	auto skybox = CreatePSODesc(_solidInputLayout, _rootSignatureDefault.Get(), SHADER_VERTEX_SKYBOX, SHADER_PIXEL_SKYBOX);
	skybox.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	skybox.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// Debug Layout
	auto debug = CreatePSODesc(_colliderDebugInputLayout, _rootSignatureDefault.Get(), SHADER_VERTEX_DEBUG, SHADER_PIXEL_DEBUG);
	{
		debug.DepthStencilState.DepthEnable = false;
		debug.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		debug.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		debug.DepthStencilState.StencilEnable = false;
		debug.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		debug.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	}

	// Shadowmap Layout
	auto shadow = CreatePSODesc(_solidInputLayout, _rootSignatureDefault.Get(), SHADER_VERTEX_SHADOW, SHADER_PIXEL_SHADOW);
	{
		shadow.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		shadow.NumRenderTargets = 0;
		shadow.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		shadow.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		shadow.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		shadow.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		shadow.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		shadow.RasterizerState.SlopeScaledDepthBias = 0.0f;
		shadow.SampleDesc.Count = 1;
		shadow.SampleDesc.Quality = 0;
	}

	// Shadowmap skinned mesh Layout
	auto skinnedShadow = CreatePSODesc(_skinnedInputLayout, _rootSignatureDefault.Get(), SHADER_VERTEX_SKINNEDSHADOW, SHADER_PIXEL_SHADOW);
	{
		skinnedShadow.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		skinnedShadow.NumRenderTargets = 0;
		skinnedShadow.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		skinnedShadow.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		skinnedShadow.RasterizerState = shadow.RasterizerState;
		skinnedShadow.SampleDesc.Count = 1;
		skinnedShadow.SampleDesc.Quality = 0;
	}

	auto terrainShadow = CreatePSODesc(_solidInputLayout, _rootSignatureTerrain.Get(), SHADER_VERTEX_TERRAINSHADOW, SHADER_PIXEL_SHADOW);
	{
		terrainShadow.RTVFormats[0] = DXGI_FORMAT_UNKNOWN; // ���̸�
		terrainShadow.NumRenderTargets = 0;
		terrainShadow.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		terrainShadow.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		terrainShadow.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		terrainShadow.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		terrainShadow.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		terrainShadow.RasterizerState.SlopeScaledDepthBias = 0.0f;
		terrainShadow.SampleDesc.Count = 1;
		terrainShadow.SampleDesc.Quality = 0;
	}

	// Terrain Layout
	auto terrain = CreatePSODesc(_solidInputLayout, _rootSignatureTerrain.Get(), SHADER_VERTEX_TERRAIN, SHADER_PIXEL_TERRAIN);

	// Particle Layout
	auto particleUpdate = CreateCSPSODesc(_rootSignatureParticle.Get(), SHADER_COMPUTE_PARTICLE);
	auto particleRender = CreatePSODesc(_rootSignatureParticle.Get(), SHADER_VERTEX_PARTICLE, SHADER_PIXEL_PARTICLE, L"", L"", SHADER_GEOMETRY_PARTICLE);
	{
		particleRender.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

		D3D12_BLEND_DESC particleBlendDesc = {};
		particleBlendDesc.AlphaToCoverageEnable = FALSE;
		particleBlendDesc.IndependentBlendEnable = FALSE;
		auto& blendRt = particleBlendDesc.RenderTarget[0];
		blendRt.BlendEnable = TRUE;
		blendRt.LogicOpEnable = FALSE;
		blendRt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendRt.DestBlend = D3D12_BLEND_ONE;
		blendRt.BlendOp = D3D12_BLEND_OP_ADD;
		blendRt.SrcBlendAlpha = D3D12_BLEND_ONE;
		blendRt.DestBlendAlpha = D3D12_BLEND_ZERO;
		blendRt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendRt.LogicOp = D3D12_LOGIC_OP_NOOP;
		blendRt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_DEPTH_STENCIL_DESC depthDesc = {};
		depthDesc.DepthEnable = TRUE;
		depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

		particleRender.BlendState = particleBlendDesc;
		particleRender.DepthStencilState = depthDesc;
	}

	// UI Layout
	auto clientUI = CreatePSODesc(_solidInputLayout, _rootSignatureUI.Get(), SHADER_VERTEX_UI, SHADER_PIXEL_UI);
	{
		clientUI.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

		clientUI.DepthStencilState.DepthEnable = FALSE;
		clientUI.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

		clientUI.BlendState.RenderTarget[0].BlendEnable = TRUE;
		clientUI.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		clientUI.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		clientUI.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		clientUI.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		clientUI.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		clientUI.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		clientUI.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		clientUI.SampleDesc.Count = 1;
		clientUI.SampleDesc.Quality = 0;
	}

	// Editor Outline
	auto outlineSolid = CreatePSODesc(_solidInputLayout, _rootSignatureDefault.Get(), SHADER_VERTEX_DEFAULT, SHADER_PIXEL_SHADOW);
	outlineSolid.RTVFormats[0] = DXGI_FORMAT_R8_UNORM;
	outlineSolid.SampleDesc.Count = 1;
	outlineSolid.SampleDesc.Quality = 0;
	outlineSolid.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	outlineSolid.DSVFormat = DXGI_FORMAT_UNKNOWN;

	auto outlineSkinned = CreatePSODesc(_skinnedInputLayout, _rootSignatureDefault.Get(), SHADER_VERTEX_SKINNED, SHADER_PIXEL_SHADOW);
	outlineSkinned.RTVFormats[0] = DXGI_FORMAT_R8_UNORM;
	outlineSkinned.SampleDesc.Count = 1;
	outlineSkinned.SampleDesc.Quality = 0;
	outlineSkinned.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	outlineSkinned.DSVFormat = DXGI_FORMAT_UNKNOWN;

	auto outlineTerrain = CreatePSODesc(_solidInputLayout, _rootSignatureTerrain.Get(), SHADER_VERTEX_TERRAIN, SHADER_PIXEL_SHADOW);
	outlineTerrain.RTVFormats[0] = DXGI_FORMAT_R8_UNORM;
	outlineTerrain.SampleDesc.Count = 1;
	outlineTerrain.SampleDesc.Quality = 0;
	outlineTerrain.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	outlineSkinned.DSVFormat = DXGI_FORMAT_UNKNOWN;

	auto postProcessing = CreatePSODesc(_solidInputLayout, _rootSignatureDefault.Get(), L"postprocessingVS", L"postprocessingPS");
	postProcessing.DepthStencilState.DepthEnable = FALSE;
	postProcessing.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	postProcessing.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	postProcessing.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	postProcessing.SampleDesc.Count = 1;
	postProcessing.SampleDesc.Quality = 0;

	BuildPSO(PSO_OPAQUE_SOLID, opaqueSolid);
	BuildPSO(PSO_OPAQUE_SKINNED, opaqueSkinned);
	BuildPSO(PSO_TRANS_SOLID, transSolid);
	BuildPSO(PSO_TRANS_SKINNED, transSkinned);
	BuildPSO(PSO_WIREFRAME, opaqueWireframe);
	BuildPSO(PSO_SKYBOX, skybox);
	BuildPSO(PSO_SHADOWMAP, shadow);
	BuildPSO(PSO_SHADOWMAP_SKINNED, skinnedShadow);
	BuildPSO(PSO_SHADOWMAP_TERRAIN, terrainShadow);
	BuildPSO(PSO_DEBUG_PHYSICS, debug);
	BuildPSO(PSO_TERRAIN, terrain);
	BuildPSO(PSO_PARTICLE_UPDATE, particleUpdate);
	BuildPSO(PSO_PARTICLE_RENDER, particleRender);
	BuildPSO(PSO_UI, clientUI);
	BuildPSO(PSO_OUTLINE_SOLID, outlineSolid);
	BuildPSO(PSO_OUTLINE_SKINNED, outlineSkinned);
	BuildPSO(PSO_OUTLINE_TERRAIN, outlineTerrain);
	BuildPSO("postprocessing", postProcessing);

	SetDefaultPSO();
}

void RenderManager::SetStateCommon(ID3D12GraphicsCommandList* cmdList)
{
	auto srvHeap = GRAPHIC->GetSRVHeap();
	UINT srvDescSize = GRAPHIC->GetCBVSRVDescriptorSize();

	ID3D12DescriptorHeap* descriptorHeaps[] = { srvHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	CD3DX12_GPU_DESCRIPTOR_HANDLE mat(srvHeap->GetGPUDescriptorHandleForHeapStart());
	mat.Offset(_currFrameResource->GetMaterialSRVHeapIndex(), srvDescSize);
	cmdList->SetGraphicsRootDescriptorTable(ROOT_PARAM_MATERIAL_SB, mat);

	CD3DX12_GPU_DESCRIPTOR_HANDLE lightDesc(srvHeap->GetGPUDescriptorHandleForHeapStart());
	lightDesc.Offset(_currFrameResource->GetLightSRVHeapIndex(), srvDescSize);
	cmdList->SetGraphicsRootDescriptorTable(ROOT_PARAM_LIGHT_SB, lightDesc);

	CD3DX12_GPU_DESCRIPTOR_HANDLE skybox(srvHeap->GetGPUDescriptorHandleForHeapStart());
	skybox.Offset(_skyboxTexSrvHeapIndex, srvDescSize);
	cmdList->SetGraphicsRootDescriptorTable(ROOT_PARAM_SKYBOX_SR, skybox);

	cmdList->SetGraphicsRootDescriptorTable(ROOT_PARAM_SHADOWMAP_SR, _shadowMap->GetSrv());

	CD3DX12_GPU_DESCRIPTOR_HANDLE tex(srvHeap->GetGPUDescriptorHandleForHeapStart());
	tex.Offset(0, srvDescSize);
	cmdList->SetGraphicsRootDescriptorTable(ROOT_PARAM_TEXTURE_ARR, tex);

	cmdList->SetGraphicsRoot32BitConstant(ROOT_PARAM_LIGHTINFO_C, _lights.size(), 0);
	cmdList->SetGraphicsRoot32BitConstant(ROOT_PARAM_LIGHTINFO_C, _iblBRDFlutTextureIdx, 1);

	cmdList->SetGraphicsRootConstantBufferView(ROOT_PARAM_CAMERA_CB, _currFrameResource->cameraCB->GetResource()->GetGPUVirtualAddress());
}

void RenderManager::SetStateDefault(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetGraphicsRootSignature(_rootSignatureDefault.Get());

	float clientInfo[2] = { TIME->DeltaTime(), TIME->TotalTime() };
	cmdList->SetGraphicsRoot32BitConstants(ROOT_PARAM_CLIENTINFO_C, 2, &clientInfo, 0);

	SetStateCommon(cmdList);

	CD3DX12_GPU_DESCRIPTOR_HANDLE instance(GRAPHIC->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart());
	instance.Offset(_currFrameResource->GetInstanceSRVHeapIndex(), GRAPHIC->GetCBVSRVDescriptorSize());
	cmdList->SetGraphicsRootDescriptorTable(ROOT_PARAM_INSTCANCE_SB, instance);
}

void RenderManager::SetStateTerrain(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetGraphicsRootSignature(_rootSignatureTerrain.Get());

	float clientInfo[2] = { TIME->DeltaTime(), TIME->TotalTime() };
	cmdList->SetGraphicsRoot32BitConstants(ROOT_PARAM_CLIENTINFO_C, 2, &clientInfo, 0);

	SetStateCommon(cmdList);
}

void RenderManager::SetStateParticle(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetGraphicsRootSignature(_rootSignatureParticle.Get());
	cmdList->SetComputeRootSignature(_rootSignatureParticle.Get());

	float clientInfo[2] = { TIME->DeltaTime(), TIME->TotalTime() };
	cmdList->SetGraphicsRoot32BitConstants(ROOT_PARAM_CLIENTINFO_C, 2, &clientInfo, 0);
	cmdList->SetComputeRoot32BitConstants(ROOT_PARAM_CLIENTINFO_C, 2, &clientInfo, 0);

	SetStateCommon(cmdList);
}

void RenderManager::SetStateUI(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetGraphicsRootSignature(_rootSignatureUI.Get());

	SetStateCommon(cmdList);

	CD3DX12_GPU_DESCRIPTOR_HANDLE uiInstances(GRAPHIC->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart());
	uiInstances.Offset(UI->GetUIBufferSRVIndex(), GRAPHIC->GetCBVSRVDescriptorSize());
	cmdList->SetGraphicsRootDescriptorTable(ROOT_PARAM_UI_SB, uiInstances);
}

#pragma endregion


std::array<const CD3DX12_STATIC_SAMPLER_DESC, STATIC_SAMPLER_COUNT> RenderManager::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC shadow(
		6, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp,
		shadow 
	};
}
