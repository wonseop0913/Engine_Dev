#include "pch.h"
#include "ResourceManager.h"

ResourceManager* ResourceManager::s_instance = nullptr;

ResourceManager::~ResourceManager()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - ResourceManager\n";
#endif

	for (KeyObjMap m : _resources) {
		for (auto r : m) {
			r.second.reset();
		}
	}
}

ResourceManager* ResourceManager::GetInstance()
{
	if (s_instance == nullptr)
		s_instance = new ResourceManager();
	return s_instance;
}

Bulb::ProcessResult ResourceManager::Delete()
{
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
		return Bulb::ProcessResult::SUCCESS;
	}
	return Bulb::ProcessResult::FAILED_INSTANCE_NOT_FOUND;
}

void ResourceManager::Init()
{
	//==========Shader==========
	D3D_SHADER_MACRO skinnedDefines[] =
	{
		"SKINNED", "1", NULL, NULL
	};

	// opaque shaders
	auto stdVS = make_shared<Shader>(L"Default\\VS.hlsl", nullptr, ShaderType::VS);
	Add<Shader>(SHADER_VERTEX_DEFAULT, stdVS);
	auto skinnedVS = make_shared<Shader>(L"Default\\VS.hlsl", skinnedDefines, ShaderType::VS);
	Add<Shader>(SHADER_VERTEX_SKINNED, skinnedVS);
	auto opaquePS = make_shared<Shader>(L"Default\\PS.hlsl", nullptr, ShaderType::PS);
	Add<Shader>(SHADER_PIXEL_DEFAULT, opaquePS);

	// skybox shaders
	auto skyboxVS = make_shared<Shader>(L"Skybox\\VS.hlsl", nullptr, ShaderType::VS);
	Add<Shader>(SHADER_VERTEX_SKYBOX, skyboxVS);
	auto skyboxPS = make_shared<Shader>(L"Skybox\\PS.hlsl", nullptr, ShaderType::PS);
	Add<Shader>(SHADER_PIXEL_SKYBOX, skyboxPS);

	// shadowmap shaders
	auto shadowVS = make_shared<Shader>(L"Shadow\\VS.hlsl", nullptr, ShaderType::VS);
	Add<Shader>(SHADER_VERTEX_SHADOW, shadowVS);
	auto skinnedShadowVS = make_shared<Shader>(L"Shadow\\VS.hlsl", skinnedDefines, ShaderType::VS);
	Add<Shader>(SHADER_VERTEX_SKINNEDSHADOW, skinnedShadowVS);
	auto shadowPS = make_shared<Shader>(L"Shadow\\PS.hlsl", nullptr, ShaderType::PS);
	Add<Shader>(SHADER_PIXEL_SHADOW, shadowPS);

	// collider debug shaders
	auto debugVS = make_shared<Shader>(L"Editor\\DebugVS.hlsl", nullptr, ShaderType::VS);
	Add<Shader>(SHADER_VERTEX_DEBUG, debugVS);
	auto debugPS = make_shared<Shader>(L"Editor\\DebugPS.hlsl", nullptr, ShaderType::PS);
	Add<Shader>(SHADER_PIXEL_DEBUG, debugPS);

	// particle shaders
	auto particleCS = make_shared<Shader>(L"Particle\\CS.hlsl", nullptr, ShaderType::CS);
	Add<Shader>(SHADER_COMPUTE_PARTICLE, particleCS);
	auto particleVS = make_shared<Shader>(L"Particle\\VS.hlsl", nullptr, ShaderType::VS);
	Add<Shader>(SHADER_VERTEX_PARTICLE, particleVS);
	auto particleGS = make_shared<Shader>(L"Particle\\GS.hlsl", nullptr, ShaderType::GS);
	Add<Shader>(SHADER_GEOMETRY_PARTICLE, particleGS);
	auto particlePS = make_shared<Shader>(L"Particle\\PS.hlsl", nullptr, ShaderType::PS);
	Add<Shader>(SHADER_PIXEL_PARTICLE, particlePS);

	// terrain shaders
	auto terrainVS = make_shared<Shader>(L"Terrain\\VS.hlsl", nullptr, ShaderType::VS);
	Add<Shader>(SHADER_VERTEX_TERRAIN, terrainVS);
	auto terrainPS = make_shared<Shader>(L"Terrain\\PS.hlsl", nullptr, ShaderType::PS);
	Add<Shader>(SHADER_PIXEL_TERRAIN, terrainPS);
	auto terrainShadowVS = make_shared<Shader>(L"Terrain\\ShadowVS.hlsl", nullptr, ShaderType::VS);
	Add<Shader>(SHADER_VERTEX_TERRAINSHADOW, terrainShadowVS);

	// ui shaders
	auto uiVS = make_shared<Shader>(L"UI\\VS.hlsl", nullptr, ShaderType::VS);
	Add<Shader>(SHADER_VERTEX_UI, uiVS);
	auto uiPS = make_shared<Shader>(L"UI\\PS.hlsl", nullptr, ShaderType::PS);
	Add<Shader>(SHADER_PIXEL_UI, uiPS);

	// PostProcessing shaders
	auto ppVS = make_shared<Shader>(L"PostProcessing\\VS.hlsl", nullptr, ShaderType::VS);
	Add<Shader>(SHADER_VERTEX_POSTPROCESSING, ppVS);
	auto ppPS = make_shared<Shader>(L"PostProcessing\\PS.hlsl", nullptr, ShaderType::PS);
	Add<Shader>(SHADER_PIXEL_POSTPROCESSING, ppPS);


	//==========Texture==========
	auto defaultTex = make_shared<Texture>(L"..\\Resources\\Textures\\EngineDefault\\white1x1.dds");
	defaultTex->SetName("Tex_Default");
	Add<Texture>(L"Tex_Default", defaultTex);

	auto skyboxTex = make_shared<Texture>(L"..\\Resources\\Textures\\EngineDefault\\Skybox_Daylight.dds", TextureType::Skybox);
	skyboxTex->SetName("Tex_DefaultSkybox");
	Add<Texture>(L"Tex_DefaultSkybox", skyboxTex);


	//==========Material==========
	auto defaultMat = make_shared<Material>("Mat_Default");
	defaultMat->SetPath("Mat_Default");
	Add<Material>(L"Mat_Default", defaultMat);


	//==========Mesh==========
	shared_ptr<Mesh> boxMesh = make_shared<Mesh>(GeometryGenerator::CreateBox(1.0f, 1.0f, 1.0f, 1));
	boxMesh->SetPath(DEFAULT_MESH_BOX);
	boxMesh->SetName(DEFAULT_MESH_BOX);
	Add<Mesh>(DEFAULT_MESH_BOX, boxMesh);

	shared_ptr<Mesh> sphereMesh = make_shared<Mesh>(GeometryGenerator::CreateGeosphere(0.5f, 3));
	sphereMesh->SetPath(DEFAULT_MESH_SPHERE);
	sphereMesh->SetName(DEFAULT_MESH_SPHERE);
	Add<Mesh>(DEFAULT_MESH_SPHERE, sphereMesh);

	shared_ptr<Mesh> quadMesh = make_shared<Mesh>(GeometryGenerator::CreateQuad());
	quadMesh->SetPath(DEFAULT_MESH_QUAD);
	quadMesh->SetName(DEFAULT_MESH_QUAD);
	Add<Mesh>(DEFAULT_MESH_QUAD, quadMesh);

	shared_ptr<Mesh> skyboxSphereMesh = make_shared<Mesh>(GeometryGenerator::CreateGeosphere(0.5f, 1));
	skyboxSphereMesh->SetPath(DEFAULT_MESH_SKYBOX);
	skyboxSphereMesh->SetName(DEFAULT_MESH_SKYBOX);
	Add<Mesh>(DEFAULT_MESH_SKYBOX, skyboxSphereMesh);
}

bool ResourceManager::CheckResourceExists(const string& filePath)
{
	return _resourcePaths.find(filePath) != _resourcePaths.end();
}

void ResourceManager::SaveMesh(shared_ptr<Mesh> mesh, const string& filePath)
{
	string finalPath = filePath == "" ? mesh->GetName() : filePath;
	mesh->SetPath(RESOURCE_PATH_MESH + finalPath + BULB_EXT_MESH);
	HANDLE fileHandle = FILEIO->CreateFileHandle<Mesh>(finalPath);

	UINT32 vertexCount = mesh->GetVertexCount();
	FILEIO->WriteToFile(fileHandle, vertexCount);
	for (const Vertex& v : mesh->GetVertices())
	{
		FILEIO->WriteToFile(fileHandle, v);
	}

	UINT32 indexCount = mesh->GetIndexCount();
	FILEIO->WriteToFile(fileHandle, indexCount);
	for (UINT32 i : mesh->GetIndices())
	{
		FILEIO->WriteToFile(fileHandle, i);
	}

	string matPath = mesh->GetMaterial()->GetPath();
	FILEIO->WriteToFile(fileHandle, matPath);

	CloseHandle(fileHandle);
}

void ResourceManager::SaveAnimation(shared_ptr<Animation> animation, const string& filePath)
{
	string finalPath = filePath == "" ? animation->GetName() : filePath;
	animation->SetPath(RESOURCE_PATH_ANIMATION + finalPath + BULB_EXT_ANIMATION);
	HANDLE fileHandle = FILEIO->CreateFileHandle<Animation>(finalPath);

	FILEIO->WriteToFile(fileHandle, animation->GetDuration());
	FILEIO->WriteToFile(fileHandle, animation->GetTicksPerSecond());

	vector<Animation::AnimationData> animationDatas = animation->GetAnimationDatas();
	FILEIO->WriteToFile(fileHandle, (UINT32)animationDatas.size());

	for (auto& animData : animationDatas)
	{
		FILEIO->WriteToFile(fileHandle, animData.boneName);

		UINT32 boneId = animData.boneId;
		FILEIO->WriteToFile(fileHandle, boneId);

		UINT32 keyFrameCount = animData.keyFrames.size();
		FILEIO->WriteToFile(fileHandle, keyFrameCount);
		for (auto& keyFrame : animData.keyFrames)
		{
			FILEIO->WriteToFile(fileHandle, keyFrame);
		}
	}

	CloseHandle(fileHandle);
}

void ResourceManager::SaveBone(map<string, BoneData> bones, const string& boneName, const string& filePath)
{
	string finalPath = filePath == "" ? boneName : filePath;
	HANDLE fileHandle = FILEIO->CreateFileHandle<BoneData>(finalPath);

	UINT32 boneCount = bones.size();
	FILEIO->WriteToFile(fileHandle, boneCount);

	for (auto& bone : bones)
	{
		FILEIO->WriteToFile(fileHandle, bone.second.name);
		FILEIO->WriteToFile(fileHandle, bone.second.id);
		FILEIO->WriteToFile(fileHandle, bone.second.parentId);
		FILEIO->WriteToFile(fileHandle, bone.second.offsetTransform);
		FILEIO->WriteToFile(fileHandle, bone.second.localBindTransform);
	}
	CloseHandle(fileHandle);
}

void ResourceManager::SavePrefab(shared_ptr<GameObject> prefabObject, const string& filePath)
{
	vector<shared_ptr<Transform>> allObjects;
	allObjects.push_back(prefabObject->GetTransform());
	for (int i = 0; i < allObjects.size(); i++)
	{
		allObjects.insert(allObjects.end(), allObjects[i]->GetChilds().begin(), allObjects[i]->GetChilds().end());
	}

	string finalPath = filePath == "" ? prefabObject->GetName() : filePath;
	HANDLE fileHandle = FILEIO->CreateFileHandle<GameObject>(prefabObject->GetName());
	
	FILEIO->WriteToFile(fileHandle, (UINT32)allObjects.size());
	SavePrefabRecursive(fileHandle, prefabObject, -1, prefabObject->GetName());
	CloseHandle(fileHandle);
}

void ResourceManager::SavePrefabRecursive(HANDLE fileHandle, shared_ptr<GameObject> object, int parentIdx, const string& prefabName)
{
	static int objectIdx = 0;
	if (parentIdx == -1)
		objectIdx = 0;

	int currentIdx = objectIdx++;

	FILEIO->WriteToFile(fileHandle, object->GetName());
	FILEIO->WriteToFile(fileHandle, object->GetPSOName());
	FILEIO->WriteToFile(fileHandle, object->GetTransform()->GetWorldMatrix());
	FILEIO->WriteToFile(fileHandle, parentIdx);		// 부모 인덱스
	FILEIO->WriteToFile(fileHandle, (UINT32)object->GetComponentCount() - 1);		// 컴포넌트 개수에 무조건 있는 트랜스폼 하나 빼기
	for (auto& componentVec : object->GetAllComponents()) {
		for (auto& c : componentVec) {
			if (c->type == ComponentType::Transform)
				continue;

			FILEIO->WriteToFile(fileHandle, c->type);	// 컴포넌트 타입

			switch (c->type)
			{
			case ComponentType::MeshRenderer:
			{
				auto meshRenderer = static_pointer_cast<MeshRenderer>(c);

				// Mesh Path
				FILEIO->WriteToFile(fileHandle, meshRenderer->GetMesh()->GetPath());

				// Material Path
				FILEIO->WriteToFile(fileHandle, meshRenderer->GetMaterial()->GetPath());

				break;
			}

			case ComponentType::SkinnedMeshRenderer:
			{
				auto meshRenderer = static_pointer_cast<SkinnedMeshRenderer>(c);

				// Mesh Path
				FILEIO->WriteToFile(fileHandle, meshRenderer->GetMesh()->GetPath());

				// Material Path
				FILEIO->WriteToFile(fileHandle, meshRenderer->GetMaterial()->GetPath());

				string rootBoneName = meshRenderer->GetRootBone()->GetGameObject()->GetName();
				FILEIO->WriteToFile(fileHandle, rootBoneName);

				string boneDataName = prefabName;
				FILEIO->WriteToFile(fileHandle, boneDataName);

				break;
			}

			case ComponentType::Animator:
			{
				auto animator = static_pointer_cast<Animator>(c);

				FILEIO->WriteToFile(fileHandle, animator->IsPlayOnInit());
				FILEIO->WriteToFile(fileHandle, animator->IsLoop());

				string currentAnimName = animator->GetCurrentAnimation()->GetName();
				FILEIO->WriteToFile(fileHandle, currentAnimName);

				auto animations = animator->GetAnimations();
				FILEIO->WriteToFile(fileHandle, (UINT32)animations.size());
				for (auto& anim : animations)
				{
					FILEIO->WriteToFile(fileHandle, anim.second->GetPath());	// ResourcePath
				}

				break;
			}

			case ComponentType::Camera:
			{
				auto camera = static_pointer_cast<Camera>(c);

				FILEIO->WriteToFile(fileHandle, camera->IsMainCamera());

				break;
			}

			case ComponentType::Script:
			{
				// 일단 씬에서 수동적으로 넣어주는 방식으로 작동중.
				// 세이브를 한다면 어떻게?
			}
			}
		}
	}

	for (auto& c : object->GetTransform()->GetChilds())
	{
		SavePrefabRecursive(fileHandle, c->GetGameObject(), currentIdx, prefabName);
	}
}

void ResourceManager::SavePrefabXML(shared_ptr<GameObject> prefabObject, const string& filePath)
{
	filesystem::path savePath(filePath + prefabObject->GetName() + ".xml");

	tinyxml2::XMLDocument doc;
	XMLElement* prefabElem = doc.NewElement("Prefab");
	doc.InsertFirstChild(prefabElem);

	prefabElem->SetAttribute("Name", prefabObject->GetName().c_str());
	prefabElem->SetAttribute("PSO", prefabObject->GetPSOName().c_str());
	prefabElem->SetAttribute("Tag", prefabObject->GetTag().c_str());

	XMLElement* compsElem = prefabElem->InsertNewChildElement("Components");
	auto& componentsArr = prefabObject->GetAllComponents();
	for (auto& componentsVec : componentsArr) {
		for (auto& component : componentsVec) {
			XMLElement* compElem = compsElem->InsertNewChildElement("Component");
			component->SaveXML(compElem);
		}
	}

	shared_ptr<Transform> transform = prefabObject->GetTransform();
	auto& childs = transform->GetChilds();
	if (childs.size() > 0) {
		XMLElement* childsElem = prefabElem->InsertNewChildElement("GameObjects");
		for (int i = 0; i < childs.size(); ++i) {
			SavePrefabXMLRecursive(childsElem, childs[i]->GetGameObject());
		}
	}

	doc.SaveFile(savePath.string().c_str());
}

void ResourceManager::SavePrefabXMLRecursive(XMLElement* objsElem, shared_ptr<GameObject> object)
{
	XMLElement* objElem = objsElem->InsertNewChildElement("GameObject");

	objElem->SetAttribute("Name", object->GetName().c_str());
	objElem->SetAttribute("PSO", object->GetPSOName().c_str());
	objElem->SetAttribute("Tag", object->GetTag().c_str());

	XMLElement* compsElem = objElem->InsertNewChildElement("Components");
	auto& componentsArr = object->GetAllComponents();
	for (auto& componentsVec : componentsArr) {
		for (auto& component : componentsVec) {
			XMLElement* compElem = compsElem->InsertNewChildElement("Component");
			component->SaveXML(compElem);
		}
	}

	shared_ptr<Transform> transform = object->GetTransform();
	auto& childs = transform->GetChilds();
	if (childs.size() > 0) {
		XMLElement* childsElem = objElem->InsertNewChildElement("GameObjects");
		for (int i = 0; i < childs.size(); ++i) {
			SavePrefabXMLRecursive(childsElem, childs[i]->GetGameObject());
		}
	}
}

void ResourceManager::LoadPrefabXMLRecursive(XMLElement* objsElem, shared_ptr<GameObject> parent)
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

				go->AddComponent(component);
				component->LoadXML(compElem);

				compElem = compElem->NextSiblingElement("Component");
			}
		}

		// Child Objects
		XMLElement* childsElem = objElem->FirstChildElement("GameObjects");
		if (childsElem != nullptr) {
			LoadPrefabXMLRecursive(childsElem, go);
		}

		if (parent != nullptr) {
			go->GetTransform()->SetParentOnly(parent->GetTransform());
		}

		objElem = objElem->NextSiblingElement("GameObject");
	}
}

shared_ptr<Mesh> ResourceManager::LoadMesh(const string& filePath)
{
	string meshName = filePath.substr(filePath.find_last_of("\\") + 1, filePath.length());
	meshName = meshName.substr(0, meshName.find_last_of('.'));

	// 좀 더 개선할 수 있을 것 같음. 나머지 리소스들도 다.
	if (RESOURCE->CheckResourceExists(filePath)) {
		return RESOURCE->Get<Mesh>(Utils::ToWString(filePath));
	}

	HANDLE fileHandle = FILEIO->CreateFileHandle<Mesh>(filePath, false);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		DEBUG->ErrorLog("Failed to load mesh file: " + filePath);
		return nullptr;
	}

	UINT32 vertexCount;
	FILEIO->ReadFileData(fileHandle, &vertexCount, sizeof(UINT32));
	vector<Vertex> vertices;
	for (int i = 0; i < vertexCount; i++)
	{
		Vertex v;
		FILEIO->ReadFileData(fileHandle, &v, sizeof(Vertex));

		vertices.push_back(v);
	}

	UINT32 indexCount;
	FILEIO->ReadFileData(fileHandle, &indexCount, sizeof(UINT32));
	vector<UINT32> indices;
	for (int i = 0; i < indexCount; i++)
	{
		// 바이너리 파일 UINT16 -> UINT32 교체작업이 이루어지지 않음.
		// 임시로 UINT16을 읽고 UINT32로 캐스팅하도록 처리해둠.
		// 후에 리소스 관련해서 일괄 수정이 반드시 필요한 부분.
		UINT16 index;
		FILEIO->ReadFileData(fileHandle, &index, sizeof(UINT16));
		indices.push_back(index);
	}

	string matPath;
	FILEIO->ReadFileData(fileHandle, matPath);

	CloseHandle(fileHandle);

	shared_ptr<Geometry> geometry = make_shared<Geometry>(vertices, indices);
	shared_ptr<Mesh> loadedMesh = make_shared<Mesh>(geometry);
	loadedMesh->SetName(meshName);
	loadedMesh->SetPath(filePath);
	loadedMesh->SetMaterial(RESOURCE->Get<Material>(Utils::ToWString(matPath)));

	Add<Mesh>(loadedMesh->GetPathW(), loadedMesh);

	return loadedMesh;
}

shared_ptr<Animation> ResourceManager::LoadAnimation(const string& filePath)
{
	string animName;
	animName = filePath.substr(filePath.find_last_of("\\") + 1, filePath.length());
	animName = animName.substr(0, animName.find_last_of('.'));

	if (RESOURCE->CheckResourceExists(filePath)) {
		return RESOURCE->Get<Animation>(Utils::ToWString(animName));
	}

	HANDLE fileHandle = FILEIO->CreateFileHandle<Animation>(filePath, false);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		DEBUG->ErrorLog("Failed to load animation file: " + filePath);
		return nullptr;
	}

	float duration, ticks;
	FILEIO->ReadFileData(fileHandle, &duration, sizeof(float));
	FILEIO->ReadFileData(fileHandle, &ticks, sizeof(float));

	// filePath는 Resources/리소스타입/ 이후의 디렉토리 경로 데이터로 생각중.
	// 따라서 filePath에서 파일 이름만 추출해서 사용하도록 변경해야함.
	// 애니메이션 뿐만 아니라 모든 리소스에대해 작업 필요.
	// ㄴ해결 된건가? 기억이 안남. 검증 필요.
	shared_ptr<Animation> loadedAnimation = make_shared<Animation>(animName, duration, ticks);
	loadedAnimation->SetPath(filePath);

	UINT32 animationDataCount;
	FILEIO->ReadFileData(fileHandle, &animationDataCount, sizeof(UINT32));
	for (int i = 0; i < animationDataCount; i++)
	{
		Animation::AnimationData animationData;

		string objName;
		FILEIO->ReadFileData(fileHandle, objName);
		animationData.boneName = objName;

		UINT32 boneId;
		FILEIO->ReadFileData(fileHandle, &boneId, sizeof(UINT32));
		animationData.boneId = boneId;

		UINT32 keyFrameCount;
		FILEIO->ReadFileData(fileHandle, &keyFrameCount, sizeof(UINT32));

		for (int j = 0; j < keyFrameCount; j++)
		{
			Animation::KeyFrame keyFrame;
			FILEIO->ReadFileData(fileHandle, &keyFrame, sizeof(Animation::KeyFrame));
			animationData.keyFrames.push_back(keyFrame);
		}

		loadedAnimation->AddAnimationData(animationData);
	}

	CloseHandle(fileHandle);

	return loadedAnimation;
}

map<string, BoneData> ResourceManager::LoadBone(const string& filePath)
{
	map<string, BoneData> boneData;

	HANDLE fileHandle = FILEIO->CreateFileHandle<BoneData>(filePath, false);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		DEBUG->ErrorLog("Failed to load bone file: " + filePath);
		return boneData;
	}

	UINT32 boneCount;
	FILEIO->ReadFileData(fileHandle, &boneCount, sizeof(UINT32));
	for (int i = 0; i < boneCount; i++)
	{
		BoneData bone;
		FILEIO->ReadFileData(fileHandle, bone.name);
		FILEIO->ReadFileData(fileHandle, &bone.id, sizeof(UINT));
		FILEIO->ReadFileData(fileHandle, &bone.parentId, sizeof(UINT));
		FILEIO->ReadFileData(fileHandle, &bone.offsetTransform, sizeof(XMFLOAT4X4));
		FILEIO->ReadFileData(fileHandle, &bone.localBindTransform, sizeof(XMFLOAT4X4));
		boneData[bone.name] = bone;
	}
	CloseHandle(fileHandle);

	return boneData;
}

vector<shared_ptr<GameObject>> ResourceManager::LoadPrefab(const string& filePath, bool isLegacyComponent)
{
	HANDLE fileHandle = FILEIO->CreateFileHandle<GameObject>(filePath, false);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		DEBUG->ErrorLog("Failed to load prefab file: " + filePath);
		return {};
	}

	vector<shared_ptr<GameObject>> loadedObjects;
	vector<pair<int, int>> objHierarchyIdxPair;
	vector<pair<shared_ptr<SkinnedMeshRenderer>, string>> boneSettingQueue;
	vector<shared_ptr<Animator>> delayedAnimators;

	UINT32 objectCount;
	FILEIO->ReadFileData(fileHandle, &objectCount, sizeof(UINT32));
	for (int i = 0; i < objectCount; i++)
	{
		shared_ptr<GameObject> go = GameObject::Instantiate();
		string name, psoName;
		FILEIO->ReadFileData(fileHandle, name);
		FILEIO->ReadFileData(fileHandle, psoName);
		go->SetName(name);
		go->SetPSOName(psoName);

		XMFLOAT4X4 worldMat;
		FILEIO->ReadFileData(fileHandle, &worldMat, sizeof(XMFLOAT4X4));
		go->GetTransform()->SetWorldMatrix(worldMat);

		int parentIdx;
		FILEIO->ReadFileData(fileHandle, &parentIdx, sizeof(int));
		objHierarchyIdxPair.push_back({ i, parentIdx });

		UINT32 componentCount;
		FILEIO->ReadFileData(fileHandle, &componentCount, sizeof(UINT32));

		for (int j = 0; j < componentCount; j++)
		{
			ComponentType componentType;
			FILEIO->ReadFileData(fileHandle, &componentType, sizeof(ComponentType));

			if (isLegacyComponent)
				componentType = MapLegacyComponentType(static_cast<UINT32>(componentType));

			switch (componentType)
			{
				case ComponentType::MeshRenderer:
				{
					shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
					go->AddComponent(meshRenderer);

					string meshPath;
					FILEIO->ReadFileData(fileHandle, meshPath);
					meshRenderer->SetMesh(LoadMesh(meshPath));

					string matName;
					FILEIO->ReadFileData(fileHandle, matName);
					meshRenderer->SetMaterial(RESOURCE->Get<Material>(Utils::ToWString(matName)));

					break;
				}

				case ComponentType::SkinnedMeshRenderer:
				{
					shared_ptr<SkinnedMeshRenderer> meshRenderer = make_shared<SkinnedMeshRenderer>();
					go->AddComponent(meshRenderer);

					string meshPath;
					FILEIO->ReadFileData(fileHandle, meshPath);
					meshRenderer->SetMesh(LoadMesh(meshPath));

					string matName;
					FILEIO->ReadFileData(fileHandle, matName);
					meshRenderer->SetMaterial(RESOURCE->Get<Material>(Utils::ToWString(matName)));

					break;
				}

				case ComponentType::Animator:
				{
					shared_ptr<Animator> animator = make_shared<Animator>();
					go->AddComponent(animator);
					delayedAnimators.push_back(animator);

					bool isPlayOnInit, isLoop;
					FILEIO->ReadFileData(fileHandle, &isPlayOnInit, sizeof(bool));
					FILEIO->ReadFileData(fileHandle, &isLoop, sizeof(bool));

					string currentAnimName;
					FILEIO->ReadFileData(fileHandle, currentAnimName);

					UINT32 animationCount;
					FILEIO->ReadFileData(fileHandle, &animationCount, sizeof(UINT32));

					for (int k = 0; k < animationCount; k++)
					{
						string animationPath;
						FILEIO->ReadFileData(fileHandle, animationPath);
						animator->AddAnimation(LoadAnimation(animationPath));
					}

					animator->SetPlayOnInit(isPlayOnInit);
					animator->SetLoop(isLoop);
					animator->SetCurrentAnimation(currentAnimName);

					break;
				}

				case ComponentType::Camera:
				{
					shared_ptr<Camera> camera = make_shared<Camera>();
					go->AddComponent(camera);

					bool isMainCamera;
					FILEIO->ReadFileData(fileHandle, &isMainCamera, sizeof(bool));
					if (isMainCamera)
						camera->SetAsMainCamera();

					break;
				}

				case ComponentType::Script:
				{

				}
			}
		}

		loadedObjects.push_back(go);
	}

	CloseHandle(fileHandle);

	for (auto& idxPair : objHierarchyIdxPair) {
		if (idxPair.second == -1)
			continue;

		loadedObjects[idxPair.first]->GetTransform()->SetParent(loadedObjects[idxPair.second]->GetTransform());
	}

	for (auto& animator : delayedAnimators) {
		// 불안정한 구현. 추후에 파일 구조를 바꾸고 반드시 리팩토링 해야함.
		animator->SetBone(loadedObjects[0]->GetName());
	}

	if (isLegacyComponent)
		SavePrefab(loadedObjects[0]);

	return loadedObjects;
}

shared_ptr<GameObject> ResourceManager::LoadPrefabXML(const string& filePath)
{
	tinyxml2::XMLDocument doc;
	XMLError e = doc.LoadFile(filePath.c_str());
	if (e != XML_SUCCESS) return nullptr;

	shared_ptr<GameObject> rootObj = GameObject::Instantiate();
	XMLElement* rootElem = doc.FirstChildElement();

	const char* name = rootElem->Attribute("Name");
	if (name != 0) rootObj->SetName(name);

	const char* psoName = rootElem->Attribute("PSO");
	if (psoName != 0) rootObj->SetPSOName(psoName);

	const char* tag = rootElem->Attribute("Tag");
	if (tag != 0) rootObj->SetTag(tag);

	// Component
	XMLElement* componentsElem = rootElem->FirstChildElement("Components");
	if (componentsElem != nullptr) {
		XMLElement* compElem = componentsElem->FirstChildElement("Component");

		while (compElem) {
			string componentType = compElem->Attribute("ComponentType");
			shared_ptr<Component> component = ComponentFactory::Create(componentType);

			rootObj->AddComponent(component);
			component->LoadXML(compElem);

			compElem = compElem->NextSiblingElement("Component");
		}
	}

	XMLElement* objsElem = rootElem->FirstChildElement("GameObjects");
	if (objsElem != nullptr)
		LoadPrefabXMLRecursive(objsElem, rootObj);

	return rootObj;
}

ComponentType ResourceManager::MapLegacyComponentType(UINT32 legacyType)
{
	return static_cast<ComponentType>(legacyType);

	switch (legacyType) {
	case 0: return ComponentType::Undefined;
	case 1: return ComponentType::Transform;
	case 2: return ComponentType::MeshRenderer;
	case 3: return ComponentType::SkinnedMeshRenderer;
	case 4: return ComponentType::Camera;
	case 5: return ComponentType::Undefined;
	case 6: return ComponentType::Rigidbody;
	case 7: return ComponentType::Light;
	case 8: return ComponentType::Animator;
	case 9: return ComponentType::Script;
	case 10: return ComponentType::ParticleEmitter;
	case 11: return ComponentType::CharacterController;
	}
}
