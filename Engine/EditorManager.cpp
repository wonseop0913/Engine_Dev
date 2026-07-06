#include "pch.h"
#include "EditorManager.h"

EditorManager* EditorManager::s_instance = nullptr;

EditorManager::~EditorManager()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - EditorManager\n";
#endif
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
