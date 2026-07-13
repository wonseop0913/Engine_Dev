#pragma once
#include "Resource.h"

#define DEFAULT_MESH_SKYBOX				L"Mesh_SkyboxSphere"
#define DEFAULT_MESH_BOX				L"Mesh_BasicBox"
#define DEFAULT_MESH_SPHERE				L"Mesh_BasicSphere"
#define DEFAULT_MESH_QUAD				L"Mesh_BasicQuad"

#define SHADER_VERTEX_DEFAULT			L"defaultVS"
#define SHADER_VERTEX_SKINNED			L"skinnedVS"
#define SHADER_VERTEX_SKYBOX			L"skyboxVS"
#define SHADER_VERTEX_SHADOW			L"shadowVS"
#define SHADER_VERTEX_SKINNEDSHADOW		L"skinnedShadowVS"
#define SHADER_VERTEX_DEBUG				L"debugVS"
#define SHADER_VERTEX_PARTICLE			L"particleVS"
#define SHADER_VERTEX_TERRAIN			L"terrainVS"
#define SHADER_VERTEX_TERRAINSHADOW		L"terrainShadowVS"
#define SHADER_VERTEX_UI				L"uiVS"
#define SHADER_VERTEX_POSTPROCESSING	L"postprocessingVS"

#define SHADER_PIXEL_DEFAULT			L"defaultPS"
#define SHADER_PIXEL_SKYBOX				L"skyboxPS"
#define SHADER_PIXEL_SHADOW				L"shadowPS"
#define SHADER_PIXEL_DEBUG				L"debugPS"
#define SHADER_PIXEL_PARTICLE			L"particlePS"
#define SHADER_PIXEL_TERRAIN			L"terrainPS"
#define SHADER_PIXEL_UI					L"uiPS"
#define SHADER_PIXEL_POSTPROCESSING		L"postprocessingPS"

#define SHADER_COMPUTE_PARTICLE			L"particleCS"

#define SHADER_GEOMETRY_PARTICLE		L"particleGS"

class Mesh;

class BULB_API ResourceManager
{
	friend class BulbApplication;
	friend class RenderManager;

private:
	ResourceManager() = default;
	~ResourceManager();

	void Init();

public:
	ResourceManager(const ResourceManager& rhs) = delete;
	ResourceManager& operator=(const ResourceManager& rhs) = delete;

	static ResourceManager* GetInstance();
	static Bulb::ProcessResult Delete();

	template<typename T>
	bool Add(const wstring& key, shared_ptr<T> resource);

	template<typename T>
	shared_ptr<T> Get(const wstring& key);

	template<typename T>
	const map<wstring, shared_ptr<Resource>> GetByType();

	template<typename T>
	ResourceType GetResourceType();

	bool CheckResourceExists(const string& filePath);

	void SaveMesh(shared_ptr<Mesh> mesh, const string& filePath = "");
	void SaveAnimation(shared_ptr<Animation> animation, const string& filePath = "");
	void SaveBone(map<string, BoneData> bones, const string& boneName, const string& filePath = "");
	void SavePrefab(shared_ptr<GameObject> prefabObject, const string& filePath = "");

	// Use On Devlope Process
	void SavePrefabXML(shared_ptr<GameObject> prefabObject, const string& filePath = RESOURCE_PATH_PREFAB);

	shared_ptr<Mesh> LoadMesh(const string& filePath);
	shared_ptr<Animation> LoadAnimation(const string& filePath);
	map<string, BoneData> LoadBone(const string& filePath);

	// Use isLegacyComponent parameter when only ComponentType enum data changed!!
	vector<shared_ptr<GameObject>> LoadPrefab(const string& filePath, bool isLegacyComponent = false);

	shared_ptr<GameObject> LoadPrefabXML(const string& filePath);

	ComponentType MapLegacyComponentType(UINT32 legacyType);

	/* FOR EDITOR ONLY */
	void LoadEntireResources();
	void LoadMeshes();

private:
	static ResourceManager* s_instance;

	using KeyObjMap = map<wstring, shared_ptr<Resource>>;
	array<KeyObjMap, RESOURCE_TYPE_COUNT> _resources;
	set<string> _resourcePaths;

	void SavePrefabRecursive(HANDLE fileHandle, shared_ptr<GameObject> object, int parentIdx, const string& prefabName);

	void SavePrefabXMLRecursive(XMLElement* objsElem, shared_ptr<GameObject> object);
	void LoadPrefabXMLRecursive(XMLElement* objsElem, shared_ptr<GameObject> parent);
};

template<typename T>
bool ResourceManager::Add(const wstring& key, shared_ptr<T> resource)
{
	ResourceType type = GetResourceType<T>();
	KeyObjMap& keyObjMap = _resources[static_cast<UINT8>(type)];

	// 중복 추가 방지
	auto found = keyObjMap.find(key);
	if (found != keyObjMap.end())
		return false;

	_resourcePaths.insert(static_pointer_cast<Resource>(resource)->GetPath());
	keyObjMap[key] = resource;
	return true;
}

template<typename T>
shared_ptr<T>
ResourceManager::Get(const wstring& key)
{
	ResourceType type = GetResourceType<T>();
	KeyObjMap& keyObjMap = _resources[static_cast<UINT8>(type)];

	auto found = keyObjMap.find(key);
	if (found != keyObjMap.end())
		return static_pointer_cast<T>(keyObjMap[key]);

	return nullptr;
}

template<typename T>
const map<wstring, shared_ptr<Resource>>
ResourceManager::GetByType()
{
	ResourceType type = GetResourceType<T>();
	return _resources[static_cast<UINT8>(type)];
	/*
	if (type != ResourceType::Undefined)
		return _resources[static_cast<UINT8>(type)];

	return nullptr;
	*/
}

template<typename T>
ResourceType ResourceManager::GetResourceType()
{
	if (is_same_v<T, Mesh>)
		return ResourceType::Mesh;
	if (is_same_v<T, Texture>)
		return ResourceType::Texture;
	if (is_same_v<T, Material>)
		return ResourceType::Material;
	if (is_same_v<T, Shader>)
		return ResourceType::Shader;

	return ResourceType::Undefined;
}