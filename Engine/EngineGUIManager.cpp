#include "pch.h"
#include "EngineGUIManager.h"

#pragma region static fields

EngineGUIManager* EngineGUIManager::s_instance = nullptr;

DescriptorHeapAllocator EngineGUIManager::_srvHeapDescAllocator;

#pragma endregion

EngineGUIManager::~EngineGUIManager()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - EngineGUIManager\n";
#endif

	// 공식 문서에서는 소멸자에 아래 함수들을 호출하라고 되있는데 이상하게 호출하면 크래시가 뜰 때가 많음
	// 일단 주석처리로 해결
	//ImGui_ImplDX12_Shutdown();
	//ImGui_ImplWin32_Shutdown();
	//ImGui::DestroyContext();
}

EngineGUIManager* EngineGUIManager::GetInstance()
{
	if (s_instance == nullptr)
		s_instance = new EngineGUIManager();
	return s_instance;
}

Bulb::ProcessResult EngineGUIManager::Delete()
{
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
		return Bulb::ProcessResult::SUCCESS;
	}
	return Bulb::ProcessResult::FAILED_INSTANCE_NOT_FOUND;
}

void EngineGUIManager::Init()
{
	_srvHeapDescAllocator.Create(GRAPHIC->GetDevice(), RENDER->GetCommonSRVHeap().Get());

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui_ImplWin32_Init(GRAPHIC->GetMainWnd());

	ImGui_ImplDX12_InitInfo init_info = {};
	init_info.Device = GRAPHIC->GetDevice();
	init_info.CommandQueue = GRAPHIC->GetCommandQueue();
	init_info.NumFramesInFlight = RENDER->GetNumFrameResources();
	init_info.RTVFormat = GRAPHIC->GetBackBufferFormat();

	RENDER->GetAndIncreaseSRVHeapIndex();
	init_info.SrvDescriptorHeap = RENDER->GetCommonSRVHeap().Get();
	init_info.SrvDescriptorAllocFn = [](
		ImGui_ImplDX12_InitInfo*,
		D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
		D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) {
			return _srvHeapDescAllocator.Alloc(out_cpu_handle, out_gpu_handle);
		};
	init_info.SrvDescriptorFreeFn = [](
		ImGui_ImplDX12_InitInfo*,
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {
			return _srvHeapDescAllocator.Free(cpu_handle, gpu_handle);
		};

	ImGui_ImplDX12_Init(&init_info);

	_guiToggleValues[TOGGLEVALUE_GUI_DEMOWINDOW] = false;
	_guiToggleValues[TOGGLEVALUE_GUI_PREFERENCES] = false;
	_guiToggleValues[TOGGLEVALUE_GUI_CONSOLE] = false;
	_guiToggleValues[TOGGLEVALUE_GUI_ENGINESTATUS] = true;
	_guiToggleValues[TOGGLEVALUE_GUI_HIERARCHY] = true;
	_guiToggleValues[TOGGLEVALUE_GUI_INSPECTOR] = true;
	_guiToggleValues[TOGGLEVALUE_GUI_RESOURCEDIR] = false;
	_guiToggleValues[TOGGLEVALUE_GUI_DEBUGSETTINGS] = false; 
}

void EngineGUIManager::Update()
{
	ToggleWindows();
	DrawGizmo();
}

void EngineGUIManager::Render(ID3D12GraphicsCommandList* cmdList)
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	{
		if (EDITOR->IsOnPlay()) {
			ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0, 0.2f, 0, 1 });
			_isEditorColorChangedOnPlay = true;
		}

		ShowMainMenuBar();

		if (_guiToggleValues[TOGGLEVALUE_GUI_DEMOWINDOW])
			ImGui::ShowDemoWindow();

		if (_guiToggleValues[TOGGLEVALUE_GUI_CONSOLE])
			ShowConsole();
		if (_guiToggleValues[TOGGLEVALUE_GUI_ENGINESTATUS])
			ShowEngineStatus();
		if (_guiToggleValues[TOGGLEVALUE_GUI_DEBUGSETTINGS])
			ShowDebugSettings();

		ShowHierarchyView();
		ShowInspectorView();
		ShowResourceDirectory();

		if (_isEditorColorChangedOnPlay) {
			ImGui::PopStyleColor();
			_isEditorColorChangedOnPlay = false;
		}
	}

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
}

void EngineGUIManager::ShowEngineStatus()
{
	ImGuiWindowFlags windowFlags = 
		ImGuiWindowFlags_NoMove | 
		ImGuiWindowFlags_AlwaysAutoResize | 
		ImGuiWindowFlags_NoSavedSettings | 
		ImGuiWindowFlags_NoFocusOnAppearing | 
		ImGuiWindowFlags_NoDecoration | 
		ImGuiWindowFlags_NoInputs;

	const int padding = 10;
	ImVec2 windowPos = ImVec2(0 + padding, 0 + padding + 18.0f);
	ImVec2 windowPivot = ImVec2(0.0f, 0.0f);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
	ImGui::SetNextWindowBgAlpha(0.35f);

	if (ImGui::Begin("Engine Status", &_guiToggleValues[TOGGLEVALUE_GUI_ENGINESTATUS], windowFlags))
	{
		ImGui::Text("FPS: %s", ENGINESTAT->GetFrameCountString().c_str());
		ImGui::Text("AVG: %s", ENGINESTAT->GetAverageFrameCountString().c_str());
	}
	ImGui::End();
}

void EngineGUIManager::ShowDebugSettings()
{
	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoSavedSettings;

	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImVec2 windowPos = ImVec2(mainViewport->Size.x / 2.0f, mainViewport->Size.y / 2.0f);
	ImVec2 windowPivot = ImVec2(0.5f, 0.5f);
	ImVec2 windowSize = ImVec2(480, 300);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
	ImGui::SetNextWindowBgAlpha(0.60f);
	ImGui::SetNextWindowSize(windowSize);

	if (ImGui::Begin("Engine Settings", &_guiToggleValues[TOGGLEVALUE_GUI_DEBUGSETTINGS], windowFlags))
	{
		bool isPysicsDebugRenderEnabled = DEBUG->IsPhysicsDebugRenderEnabled();
		if (ImGui::Checkbox("Pysics Debug Render", &isPysicsDebugRenderEnabled))
			DEBUG->SetPhysicsDebugRenderEnabled(isPysicsDebugRenderEnabled);
	}
	ImGui::End();
}

void EngineGUIManager::ShowHierarchyView()
{
	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_HorizontalScrollbar;

	ImGuiViewport* mainViewport = ImGui::GetMainViewport();

	ImVec2 windowPos = ImVec2(mainViewport->Size.x - WIDTH_GUI_INSPECTOR, 18.0f);
	ImVec2 windowPivot = ImVec2(1.0f, 0.0f);
	ImVec2 windowSize = ImVec2(WIDTH_GUI_HIERARCHY, mainViewport->Size.y * 0.6f);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize(windowSize);

	ImGui::SetNextWindowCollapsed(!_guiToggleValues[TOGGLEVALUE_GUI_HIERARCHY]);

	if (ImGui::Begin("Hierarchy View", nullptr, windowFlags))
	{
		if (ImGui::IsWindowHovered()) {
			if (ImGui::IsMouseClicked(0)) {
				if (_selectedObj != nullptr)
					_selectedObj = nullptr;
			}
			if (ImGui::IsMouseClicked(1)) {
				ImGui::OpenPopup("hierarchy_funcs");
			}
		}

		if (ImGui::BeginPopup("hierarchy_funcs")) {
			if (ImGui::Selectable("Create Object")) {
				_selectedObj = GameObject::Instantiate();
			}
			if (ImGui::BeginMenu("Instantiate Prefab")) {
				vector<string> prefabDirectories = EDITOR->GetPrefabList();
				for (string& prefabDir : prefabDirectories) {
					if (ImGui::MenuItem(prefabDir.c_str())) {
						GameObject::LoadPrefab(prefabDir);
						break;
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}

		_guiToggleValues[TOGGLEVALUE_GUI_HIERARCHY] = true;

		auto& objects = RENDER->GetObjects();
		for (auto& o : objects) {
			if (o->GetTag() == "EditorCamera")
				continue;
			if (o->GetTransform()->GetDepthLevel() > 0)
				continue;
			HierarchyObjectRecursion(o->GetTransform());
		}
	}
	else if (_guiToggleValues[TOGGLEVALUE_GUI_HIERARCHY]) {
		_guiToggleValues[TOGGLEVALUE_GUI_HIERARCHY] = false;
	}
	ImGui::End();
}

void EngineGUIManager::ShowInspectorView()
{
	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing;

	ImGuiViewport* mainViewport = ImGui::GetMainViewport();

	ImVec2 windowPos = ImVec2(mainViewport->Size.x, 18.0f);
	ImVec2 windowPivot = ImVec2(1.0f, 0.0f);
	ImVec2 windowSize = ImVec2(WIDTH_GUI_INSPECTOR, mainViewport->Size.y * 0.6f);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize(windowSize);

	ImGui::SetNextWindowCollapsed(!_guiToggleValues[TOGGLEVALUE_GUI_INSPECTOR]);
	if (ImGui::Begin("Inspector View", nullptr, windowFlags)) {

		_guiToggleValues[TOGGLEVALUE_GUI_INSPECTOR] = true;
		if (_selectedObj == nullptr) {
			ImGui::Text("No GameObject Selected");
		}
		else {
			bool isActive = _selectedObj->IsActive();
			if (ImGui::Checkbox("Active", &isActive)) {
				_selectedObj->SetActive(isActive);
			}

			string name = _selectedObj->GetName();
			if (ImGui::InputText("ObjName", &name, ImGuiInputTextFlags_EnterReturnsTrue)) {
				_selectedObj->SetName(name);
			}
			ImGui::Text(("PSO: " + _selectedObj->GetPSOName()).c_str());
			ImGui::Text(("Tag: " + _selectedObj->GetTag()).c_str());
			if (ImGui::Button("Set Parent")) {
				_isParentSelectMode = !_isParentSelectMode;
				cout << _isParentSelectMode << endl;
			}
			ImGui::Separator();

			// Transform
			_selectedObj->GetTransform()->ShowComponentEditorGUI();
		
			// Other Components
			auto& allComponents = _selectedObj->GetAllComponents();
			for (auto& componentVec : allComponents) {
				if (componentVec.size() == 0) continue;

				for (int i = 0; i < componentVec.size(); ++i) {
					ComponentType componentType = componentVec[i]->type;
					if (componentType == ComponentType::Transform) continue;

					if (componentVec[i]->ShowComponentEditorGUI()) {
						_selectedObj->DeleteComponent(componentVec[i]);
						--i;
					}
				}
			}

			ImGui::Separator();

			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("component_select_popup");

			if (ImGui::BeginPopup("component_select_popup")) {
				auto usableCompCount = magic_enum::enum_count<UsableComponentType>();
				auto compTypeNames = magic_enum::enum_names<UsableComponentType>();

				for (int i = 0; i < usableCompCount; ++i) {
					if (ImGui::Selectable(compTypeNames[i].data())) {
						_selectedObj->AddComponent(ComponentFactory::Create(compTypeNames[i].data()));
					}
				}

				ImGui::EndPopup();
			}
		}
	}
	else if (_guiToggleValues[TOGGLEVALUE_GUI_INSPECTOR])
	{
		_guiToggleValues[TOGGLEVALUE_GUI_INSPECTOR] = false;
	}
	ImGui::End();
}

void EngineGUIManager::ShowResourceDirectory()
{
	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing;

	ImGuiViewport* mainViewport = ImGui::GetMainViewport();

	ImVec2 windowPos = ImVec2(0, mainViewport->Size.y);
	ImVec2 windowPivot = ImVec2(0.0f, 1.0f);
	ImVec2 windowSize = ImVec2(mainViewport->Size.x, HEIGHT_GUI_RESOURCEDIR);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowSize(windowSize);

	ImGui::SetNextWindowCollapsed(!_guiToggleValues[TOGGLEVALUE_GUI_RESOURCEDIR]);

	if (ImGui::Begin("Resource Directory View", nullptr, windowFlags))
	{
		_guiToggleValues[TOGGLEVALUE_GUI_RESOURCEDIR] = true;

		if (ImGui::Button("Import Asset"))
		{

		}
	}
	else if (_guiToggleValues[TOGGLEVALUE_GUI_RESOURCEDIR])
	{
		_guiToggleValues[TOGGLEVALUE_GUI_RESOURCEDIR] = false;
	}
	ImGui::End();
}

void EngineGUIManager::HierarchyObjectRecursion(shared_ptr<Transform> transform)
{
	if (transform->GetGameObject() == nullptr) return;

	ImGuiTreeNodeFlags tree_flags = 
		ImGuiTreeNodeFlags_OpenOnArrow | 
		ImGuiTreeNodeFlags_OpenOnDoubleClick | 
		ImGuiTreeNodeFlags_NavLeftJumpsBackHere;

	if (transform->GetChilds().size() == 0)
		tree_flags |= ImGuiTreeNodeFlags_Leaf;
	if (_selectedObj == transform->GetGameObject())
		tree_flags |= ImGuiTreeNodeFlags_Selected;

	bool isNodeOpen = ImGui::TreeNodeEx((transform->GetGameObject()->GetName() + "##" + to_string(transform->GetGameObject()->GetId())).c_str(), tree_flags);
	// left click
	if (ImGui::IsItemClicked(0)) {
		if (_isParentSelectMode && _selectedObj != nullptr) {
			_selectedObj->GetTransform()->SetParent(transform);
			_isParentSelectMode = false;
			cout << _isParentSelectMode << endl;
		}
		_selectedObj = transform->GetGameObject();
	}
	// right click
	else if (ImGui::IsItemClicked(1)) {
		_selectedObj = transform->GetGameObject();
		ImGui::OpenPopup("ObjectRightClickPopup");
	}
	if (ImGui::BeginPopup("ObjectRightClickPopup")) {
		if (ImGui::Selectable("Delete ##Object")) {
			_selectedObj->Delete();
			_selectedObj = nullptr;
		}
		if (ImGui::Selectable("Duplicate ##Object")) {
			_selectedObj = _selectedObj->Duplicate();
			_selectedObj->SetName(_selectedObj->GetName() + " (copy)");
		}
		if (ImGui::Selectable("Save as Prefab ##Object")) {
			RESOURCE->SavePrefabXML(_selectedObj);
		}
		ImGui::EndPopup();
	}

	if (isNodeOpen) {
		auto& childs = transform->GetChilds();
		for (shared_ptr<Transform> child : childs) {
			HierarchyObjectRecursion(child);
		}
		ImGui::TreePop();
	}
}

void EngineGUIManager::ShowMainMenuBar()
{
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			ShowMenuFile();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit")) {
			ShowMenuEdit();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Preferences")) {
			ShowPreferences();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			ShowView();
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void EngineGUIManager::ShowMenuFile()
{
	if (ImGui::MenuItem("Load Scene", 0, nullptr, !EDITOR->IsOnPlay())) {
		SCENE->LoadScene();
	}

	if (ImGui::MenuItem("Save Scene", 0, nullptr, !EDITOR->IsOnPlay())) {
		SCENE->SaveScene();
	}

	if (ImGui::MenuItem("Save As..", 0, nullptr, !EDITOR->IsOnPlay())) {
		SCENE->SaveScene(true);
	}
}

void EngineGUIManager::ShowMenuEdit()
{
	static bool isOnPlay;
	isOnPlay = EDITOR->IsOnPlay();
	if (ImGui::MenuItem("Play", 0, nullptr, !isOnPlay)) {
		EDITOR->Play();
	}

	if (ImGui::MenuItem("Stop", 0, nullptr, isOnPlay)) {
		EDITOR->Stop();
	}
}

void EngineGUIManager::ShowPreferences()
{
	if (ImGui::MenuItem("None Mouse Mode", 0, &EDITOR->isNoneMouseMode)) {
		// EditorSetting.ini overwrite
	}
}

void EngineGUIManager::ShowView()
{
	if (ImGui::BeginMenu("Debug")) {
		static bool isPysicsDebugRenderEnabled;
		isPysicsDebugRenderEnabled = DEBUG->IsPhysicsDebugRenderEnabled();
		if (ImGui::MenuItem("Collider", 0, &isPysicsDebugRenderEnabled)) {
			DEBUG->SetPhysicsDebugRenderEnabled(isPysicsDebugRenderEnabled);
		}
		ImGui::EndMenu();
	}
}

void EngineGUIManager::DrawGizmo()
{
	if (_selectedObj != nullptr) {
		shared_ptr<Transform> transform = _selectedObj->GetTransform();
		Bulb::Vector3 pos = transform->GetPosition();
		DEBUG->DrawLine(pos, pos + transform->GetRight().Normalize(), { 1.0f, 0.0f, 0.0f, 1.0f });
		DEBUG->DrawLine(pos, pos + transform->GetUp().Normalize(), { 0.0f, 1.0f, 0.0f, 1.0f });
		DEBUG->DrawLine(pos, pos + transform->GetLook().Normalize(), { 0.0f, 0.0f, 1.0f, 1.0f });
	}
}

void EngineGUIManager::ToggleWindows()
{
	if (INPUTM->IsKeyDown(KeyValue::F1)) {
		_guiToggleValues[TOGGLEVALUE_GUI_CONSOLE] = !_guiToggleValues[TOGGLEVALUE_GUI_CONSOLE];
	}

	// FPS status view
	if (INPUTM->IsKeyDown(KeyValue::F2)) {
		_guiToggleValues[TOGGLEVALUE_GUI_ENGINESTATUS] = !_guiToggleValues[TOGGLEVALUE_GUI_ENGINESTATUS];
	}

	if (INPUTM->IsKeyDown(KeyValue::F3)) {
		_guiToggleValues[TOGGLEVALUE_GUI_DEBUGSETTINGS] = !_guiToggleValues[TOGGLEVALUE_GUI_DEBUGSETTINGS];
	}

	if (INPUTM->IsKeyDown(KeyValue::F4)) {
		_guiToggleValues[TOGGLEVALUE_GUI_HIERARCHY] = !_guiToggleValues[TOGGLEVALUE_GUI_HIERARCHY];
	}
	
	if (INPUTM->IsKeyDown(KeyValue::F5)) {
		_guiToggleValues[TOGGLEVALUE_GUI_INSPECTOR] = !_guiToggleValues[TOGGLEVALUE_GUI_INSPECTOR];
	}

	if (INPUTM->IsKeyDown(KeyValue::F6)) {
		_guiToggleValues[TOGGLEVALUE_GUI_RESOURCEDIR] = !_guiToggleValues[TOGGLEVALUE_GUI_RESOURCEDIR];
	}
}

void EngineGUIManager::ShowConsole()
{
	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoSavedSettings;

	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImVec2 windowPos = ImVec2(0, 0);
	ImVec2 windowPivot = ImVec2(0, 0);
	ImVec2 windowSize = ImVec2(480, 300);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
	ImGui::SetNextWindowBgAlpha(0.60f);
	ImGui::SetNextWindowSize(windowSize);

	if (ImGui::Begin("Engine Console", &_guiToggleValues[TOGGLEVALUE_GUI_CONSOLE], windowFlags))
	{
		auto logs = DEBUG->GetLogs();
		for (const auto& log : logs)
		{
			ImVec4 textColor;
			switch(log.logLevel)
			{
				case LogLevel::LOG_INFO:
					textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
					break;
				case LogLevel::LOG_WARN:
					textColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
					break;
				case LogLevel::LOG_ERROR:
					textColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
					break;
			}
			ImGui::TextColored(textColor, "%.2f: %s", log.time, log.message.c_str());
		}
	}
	ImGui::End();
}

