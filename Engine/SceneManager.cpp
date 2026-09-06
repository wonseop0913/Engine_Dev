#include "pch.h"
#include "SceneManager.h"

SceneManager* SceneManager::s_instance = nullptr;

SceneManager::~SceneManager()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - SceneManager\n";
#endif
}

SceneManager* SceneManager::GetInstance()
{
	if (s_instance == nullptr)
		s_instance = new SceneManager();
	return s_instance;
}

Bulb::ProcessResult SceneManager::Delete()
{
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
		return Bulb::ProcessResult::SUCCESS;
	}
	return Bulb::ProcessResult::FAILED_INSTANCE_NOT_FOUND;
}

void SceneManager::Init()
{
	_isInitialized = true;

	if (_queuedScenePath != "") {
		LoadScene(_queuedScenePath);
		_queuedScenePath = "";
	}
}

void SceneManager::PreUpdate()
{
	if (_queuedScenePath != "") {
		LoadScene(_queuedScenePath);
		_queuedScenePath = "";
	}
}

void SceneManager::LoadScene()
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	ComPtr<IFileOpenDialog> pFileOpen;
	ThrowIfFailed(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pFileOpen)));

	COMDLG_FILTERSPEC fileTypes[] = {
		{ L"Scene Files (*.xml)", L"*.xml"},
		{ L"All Files (*.*)", L"*.*" }
	};
	pFileOpen->SetFileTypes(ARRAYSIZE(fileTypes), fileTypes);
	HRESULT hr = pFileOpen->Show(GRAPHIC->GetMainWnd());

	if (SUCCEEDED(hr)) {
		ComPtr<IShellItem> pItem;
		ThrowIfFailed(pFileOpen->GetResult(&pItem));

		PWSTR pszFilePath;
		ThrowIfFailed(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath));

		wstring filePath = pszFilePath;
		CoTaskMemFree(pszFilePath);

		string relPath = filesystem::relative(filePath, filesystem::path(RESOURCE_PATH_SCENE)).string();
		if (relPath.substr(0, 2) == "..") {
			DEBUG->ErrorLog("Failed make relative path of target scene! Target scene should located in Resources/Scenes folder!");
		}
		else {
			LoadSceneOnRender(relPath);
			//LoadScene(relPath);
		}
	}

	CoUninitialize();
}

void SceneManager::LoadScene(string sceneName, bool isFullPath)
{
	if (!_isInitialized) {
		_queuedScenePath = sceneName;
		return;
	}

	string path = isFullPath ? sceneName : RESOURCE_PATH_SCENE + sceneName;

	if (!filesystem::exists(path))
		return;

	if (_currentScenePath != "") {
		InitializeScene();
	}

	_currentSceneName = Utils::GetFileName(sceneName);
	_currentScenePath = path;

	tinyxml2::XMLDocument doc;
	XMLError e = doc.LoadFile(_currentScenePath.c_str());
	if (e != XML_SUCCESS)
		return;

	PHYSICS->Initialize();
	UI->Initialize();

	XMLElement* node = doc.FirstChildElement();
	RENDER->SetIBLFactor(node->FloatAttribute("IBLFactor", 0.3f));

	XMLElement* skyboxElem = node->FirstChildElement("Skybox");
	if (skyboxElem) {
		const char* skyboxTexPath = skyboxElem->Attribute("Texture");
		if (skyboxTexPath != 0) {
			shared_ptr<Texture> skyboxTex = RESOURCE->Get<Texture>(Utils::ToWString(skyboxTexPath));
			if (skyboxTex != nullptr)
				RENDER->SetSkyboxTexture(skyboxTex);
		}
	}

	XMLElement* objsElem = node->FirstChildElement("GameObjects");
	
	if (objsElem != nullptr) {
		ReadGameObjectData(objsElem, nullptr);
	}

	XMLElement* uisElem = node->FirstChildElement("UIs");

	if (uisElem != nullptr) {
		ReadUIData(uisElem, nullptr);
	}

	TIME->Reset();

#ifdef BULB_EDITOR
	EDITOR->SetEditorWindowText(_currentSceneName);
#endif
}

void SceneManager::LoadSceneOnRender(string scenePath)
{
	_queuedScenePath = scenePath;
}

void SceneManager::SaveScene(bool saveAs)
{
	if (!saveAs && !_currentScenePath.empty() && filesystem::exists(_currentScenePath)) {
		filesystem::path p(_currentScenePath);
		SaveScene(_currentScenePath, p.is_relative());	// 로드된 씬들은 이론적으로 다 상대경로긴 한데 혹시 몰라서 검증부분 넣어둠
		return;
	}

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	ComPtr<IFileSaveDialog> pFileSave;
	ThrowIfFailed(CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pFileSave)));

	// 탐색기 열 때 보이는 기본 폴더 설정하는 부분 구현해야함.
	//pFileSave->SetFolder();
	pFileSave->SetFileName(Utils::ToWString(_currentSceneName).c_str());
	pFileSave->SetDefaultExtension(L"xml");

	COMDLG_FILTERSPEC fileTypes[] = {
		{ L"Scene Files (*.xml)", L"*.xml"},
	};
	pFileSave->SetFileTypes(ARRAYSIZE(fileTypes), fileTypes);

	DWORD dwFlags;
	pFileSave->GetOptions(&dwFlags);
	pFileSave->SetOptions(dwFlags | FOS_OVERWRITEPROMPT | FOS_PATHMUSTEXIST);

	if (SUCCEEDED(pFileSave->Show(GRAPHIC->GetMainWnd()))) {
		ComPtr<IShellItem> pItem;
		ThrowIfFailed(pFileSave->GetResult(&pItem));

		PWSTR pszFilePath;
		ThrowIfFailed(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath));

		wstring filePath = pszFilePath;
		CoTaskMemFree(pszFilePath);

		string relPath = filesystem::relative(filePath, filesystem::path(RESOURCE_PATH_SCENE)).string();
		if (relPath.substr(0, 2) == "..") {
			DEBUG->ErrorLog("Failed make relative path of target scene! Target directory should located in Resources/Scenes folder!");
		}
		else {
			_currentSceneName = Utils::GetFileName(relPath);
			SaveScene(relPath);
		}
	}

	CoUninitialize();
}

void SceneManager::SaveScene(string scenePath, bool isFullPath)
{
	string path = isFullPath ? scenePath : RESOURCE_PATH_SCENE + scenePath;

	tinyxml2::XMLDocument doc;

	XMLElement* sceneElem = doc.NewElement("Scene");
	sceneElem->SetAttribute("Name", _currentSceneName.c_str());
	doc.InsertFirstChild(sceneElem);
	
	sceneElem->SetAttribute("IBLFactor", RENDER->GetIBLFactor());

	XMLElement* skyboxElem = sceneElem->InsertNewChildElement("Skybox");
	shared_ptr<Texture> skyboxTex = RENDER->GetSkyboxTexture();
	if (skyboxTex != nullptr)
		skyboxElem->SetAttribute("Texture", skyboxTex->GetPath().c_str());

	XMLElement* objsElem = sceneElem->InsertNewChildElement("GameObjects");
	auto& gameObjects = RENDER->GetObjects();
	for (auto& go : gameObjects) {
		if (go->GetTransform()->GetDepthLevel() == 0) {
#ifdef BULB_EDITOR
			if (go->GetTag() == "EditorCamera")
				continue;
#endif
			WriteGameObjectData(objsElem, go);
		}
	}

	doc.SaveFile(path.c_str());
}

void SceneManager::InitializeScene()
{
	RENDER->InitializeOnRuntime();
	ANIMATION->InitializeOnRuntime();
}

void SceneManager::ReadGameObjectData(XMLElement* objsElem, shared_ptr<GameObject> parent)
{
	XMLElement* objElem = objsElem->FirstChildElement("GameObject");

	while (objElem) {
		shared_ptr<GameObject> go;
		const char* prefabPath = objElem->Attribute("PrefabPath");

		if (prefabPath != 0) {
			go = GameObject::LoadPrefab(prefabPath);
		}
		else {
			go = GameObject::Instantiate();
		}

		const char* name = objElem->Attribute("Name");
		if (name != 0) go->SetName(name);

		const char* psoName = objElem->Attribute("PSO");
		if (psoName != 0) go->SetPSOName(psoName);

		const char* tag = objElem->Attribute("Tag");
		if (tag != 0) go->SetTag(tag);

		// Component
		XMLElement* componentsElem = objElem->FirstChildElement("Components");
		if (componentsElem != nullptr) {
			XMLElement* compElem = componentsElem->FirstChildElement("Component");

			while (compElem) {
				string componentType = compElem->Attribute("ComponentType");
				shared_ptr<Component> component = ComponentFactory::Create(componentType);

				component = go->AddComponent(component);
				component->LoadXML(compElem);

				compElem = compElem->NextSiblingElement("Component");
			}
		}

		// Child Objects
		XMLElement* childsElem = objElem->FirstChildElement("GameObjects");
		if (childsElem != nullptr) {
			ReadGameObjectData(childsElem, go);
		}

		if (parent != nullptr) {
			go->GetTransform()->SetParentOnly(parent->GetTransform());
		}

		objElem = objElem->NextSiblingElement("GameObject");
	}
}

void SceneManager::WriteGameObjectData(XMLElement* objsElem, shared_ptr<GameObject> go)
{
	XMLElement* objElem = objsElem->InsertNewChildElement("GameObject");

	if (go->IsPrefab()) {
		objElem->SetAttribute("PrefabPath", go->GetPrefabPath().c_str());
	}

	objElem->SetAttribute("Name", go->GetName().c_str());
	objElem->SetAttribute("PSO", go->GetPSOName().c_str());
	objElem->SetAttribute("Tag", go->GetTag().c_str());

	// 프리팹이 아닌 경우 -> 모든 컴포넌트 정보 저장
	// 프리팹인 경우 -> Transform의 정보 변화만 저장
	XMLElement* compsElem = objElem->InsertNewChildElement("Components");
	if (true) {
		auto& componentsArr = go->GetAllComponents();
		for (auto& componentsVec : componentsArr) {
			for (auto& component : componentsVec) {
				if (component->IsPrefabOriginated())
					continue;

				XMLElement* compElem = compsElem->InsertNewChildElement("Component");
				component->SaveXML(compElem);
			}
		}

		if (!go->IsPrefab()) {
			shared_ptr<Transform> transform = go->GetTransform();
			auto& childs = transform->GetChilds();
			if (childs.size() > 0) {
				XMLElement* childsElem = objElem->InsertNewChildElement("GameObjects");
				for (int i = 0; i < childs.size(); ++i) {
					WriteGameObjectData(childsElem, childs[i]->GetGameObject());
				}
			}
		}
	}

	objsElem->InsertEndChild(objElem);
}

void SceneManager::ReadUIData(XMLElement* uisElem, shared_ptr<UIElement> parent)
{
	XMLElement* uiElem = uisElem->FirstChildElement("UI");

	while (uiElem) {
		shared_ptr<UIElement> ui;
		string uiType(uiElem->Attribute("Type"));

		if (uiType == "Panel") ui = UI->CreateUI<UIPanel>();
		if (uiType == "Button") ui = UI->CreateUI<UIButton>();

		ui->LoadXML(uiElem);

		uiElem = uiElem->NextSiblingElement();
	}
}
