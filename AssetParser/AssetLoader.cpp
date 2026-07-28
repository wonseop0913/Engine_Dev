#include "pch.h"
#include "AssetLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

AssetLoader::AssetLoader()
{

}

AssetLoader::~AssetLoader()
{

}

void AssetLoader::InitializeFields()
{
	_nodes.clear();
	_bones.clear();
	_meshObjs.clear();
	_boneObjs.clear();
	_tempBoneWeights.clear();
	_animations.clear();
}

// 바이너리 파일로 저장 안된경우 최초 임포트 메소드
void AssetLoader::ImportAssetFile(wstring file)
{
	_importer = make_shared<Assimp::Importer>();
	_importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);	// 자꾸 이상하고 쓸데 없는 노드 가져와서 설정함

	wstring fileStr;
	if (file.find(_assetPath) != wstring::npos)
		fileStr = file;
	else
		fileStr = _assetPath + file;

	auto p = filesystem::path(fileStr);
	assert(filesystem::exists(p));
	_scene = _importer->ReadFile(
		Utils::ToString(fileStr),
		aiProcess_ConvertToLeftHanded |
		aiProcess_Triangulate |
		aiProcess_GenUVCoords |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_SortByPType |
		aiProcess_OptimizeMeshes |
		aiProcess_ValidateDataStructure |
		aiProcess_LimitBoneWeights |
		aiProcess_JoinIdenticalVertices |
		aiProcess_FlipWindingOrder
	);

	assert(_scene != nullptr);

	ImportModelFormat(Utils::ToWString(p.filename().string()));
	shared_ptr<GameObject> rootObj = make_shared<GameObject>();
	// rootObj->GetName() = UniversalUtils::ToString(_assetNameW);
	_loadedObject.push_back(rootObj);

	ProcessMaterials(_scene);
	ProcessNodes(_scene->mRootNode, _scene, nullptr);
	if (_bones.size() > 0)
	{
		MapWeights();
		MapBones();
		BuildBones();
	}
	ProcessAnimation(_scene);

	// 바이너리 파일 저장
	{
		string assetNameStr = Utils::ToString(_assetNameW);

		if (_meshes.size() > 0)
		{
			for (auto& mesh : _meshes)
			{
				RESOURCE->SaveMesh(mesh, assetNameStr + "\\" + mesh->GetName());
				cout << "Mesh parsed at " << assetNameStr + "\\" + mesh->GetName() << endl;
			}
			cout << _meshes.size() << " meshes parsed" << endl << endl;
		}

		if (_animations.size() > 0)
		{
			LoadBones();

			for (auto& animation : _animations)
			{
				RESOURCE->SaveAnimation(animation, assetNameStr + "\\" + animation->GetName());
				cout << "Animation parsed at " << assetNameStr + "\\" + animation->GetName() << endl;
			}
			cout << _animations.size() << " animations parsed" << endl << endl;
		}
		
		_loadedObject[0]->GetTransform()->ForceUpdateTransform();

		if (_bones.size() > 0 && !_isExternalBone)
		{
			RESOURCE->SaveBone(_bones, assetNameStr);
			cout << "Bone parsed at " << assetNameStr << endl << endl;
		}

		if (_loadedObject.size() > 1)
		{
			_loadedObject[0]->SetName(Utils::ToString(_assetNameW));
			// RESOURCE->SavePrefab(_loadedObject[0]);
			RESOURCE->SavePrefabXML(_loadedObject[0]);
			cout << "Prefab parsed" << endl << endl;
		}
	}

	InitializeFields();

	_importer = nullptr;
}

void AssetLoader::ProcessMaterials(const aiScene* scene)
{
	// 텍스처 로드 부분
	for (UINT i = 0; i < scene->mNumTextures; i++)
	{
		aiTexture* aiTex = scene->mTextures[i];

		if (aiTex->mHeight == 0)
		{
			int width, height, channels;
			unsigned char* decodedData = stbi_load_from_memory(
				reinterpret_cast<unsigned char*>(aiTex->pcData),
				aiTex->mWidth,
				&width,
				&height,
				&channels,
				4);

			string textureFullPath = RESOURCE_PATH_TEXTURE;
			string texturePathStr(aiTex->mFilename.C_Str());
			texturePathStr = texturePathStr.substr(texturePathStr.find_last_of('/') + 1);
			textureFullPath += texturePathStr;

			// 이미 존재하는지 확인
			if (filesystem::exists(filesystem::path(textureFullPath)))
			{
				stbi_image_free(decodedData);
				continue;
			}

			stbi_write_png(textureFullPath.c_str(), width, height, 4, decodedData, width * 4);
			stbi_image_free(decodedData);
		}

		// 비압축 raw 데이터의 경우
		else
		{

		}
	}

	// 머터리얼 로드 부분
	for (UINT i = 0; i < scene->mNumMaterials; i++)
	{
		wstring matName = GetAIMaterialName(scene, i);
		if (RESOURCE->Get<Material>(matName) != nullptr)
			continue;

		aiMaterial* aiMat = scene->mMaterials[i];
		string matNameStr(scene->mMaterials[i]->GetName().C_Str());

		shared_ptr<Material> mat = make_shared<Material>(matNameStr);

		aiColor4D color;
		aiMat->Get(AI_MATKEY_COLOR_AMBIENT, color);
		mat->ambient = { color.r, color.g, color.b, color.a };
		aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		mat->diffuse = { color.r, color.g, color.b, color.a };
		aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color);
		mat->specular = { color.r, color.g, color.b, color.a };
		aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, color);
		mat->emissive = { color.r, color.g, color.b, color.a };
		
		bool isIncludeTexture = false;

		if (!isIncludeTexture)
		{
			if (Texture::IsTextureExists(matName + L".dds"))
			{
				shared_ptr<Texture> texture = make_shared<Texture>(matName + L".dds");
				RESOURCE->Add<Texture>(matName, texture);
				mat->SetDiffuse(RESOURCE->Get<Texture>(matName));
			}
		}
		RESOURCE->Add<Material>(matName, mat);

		FILEIO->XMLFromMaterial(mat, _assetNameW);
	}
}

void AssetLoader::ProcessNodes(aiNode* node, const aiScene* scene, shared_ptr<NodeTempData> parentNode)
{
	// 노드 저장
	shared_ptr<NodeTempData> currNode = make_shared<NodeTempData>();
	currNode->name = node->mName.C_Str();
	currNode->id = _nodes.size();
	currNode->parent = parentNode;
	currNode->transform = ConvertToXMFLOAT4X4(node->mTransformation);


	// 부모 노드 존재시 행렬 계산
	//if (currNode->parent != nullptr)
	//{
	//	XMMATRIX multipliedMat = XMLoadFloat4x4(&currNode->transform) * XMLoadFloat4x4(&parentNode->transform);
	//	XMStoreFloat4x4(&currNode->transform, multipliedMat);
	//}

	_nodes.insert({ currNode->name, currNode });

	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		// 메시 기하정보 로드
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		shared_ptr<Mesh> m = RESOURCE->Get<Mesh>(Utils::ToWString(mesh->mName.C_Str()));
		if (m == nullptr)
		{
			m = ProcessMesh(mesh, scene);
			RESOURCE->Add<Mesh>(Utils::ToWString(node->mName.C_Str()), m);
		}
		_meshes.push_back(m);

		// 본 없는 경우에는 그냥 MeshRenderer로 하도록 변경 필요
		shared_ptr<GameObject> meshObj = make_shared<GameObject>();
		meshObj->SetName(Utils::ToString(m->GetNameW()));
		if (mesh->HasBones())
		{
			meshObj->AddComponent(ComponentFactory::Create("SkinnedMeshRenderer"));
			meshObj->GetComponent<SkinnedMeshRenderer>()->SetMeshPlain(m);
		}
		else
		{
			meshObj->AddComponent(ComponentFactory::Create("MeshRenderer"));
			meshObj->GetComponent<MeshRenderer>()->SetMeshPlain(m);
		}
		meshObj->GetTransform()->SetParent(_loadedObject[0]->GetTransform());
		_meshObjs.push_back(meshObj);
		_loadedObject.push_back(meshObj);

		// 메시 본 로드 (있는 경우에만)
		if (mesh->HasBones())
		{
			for (int i = 0; i < mesh->mNumBones; i++)
			{
				aiBone* currentBone = mesh->mBones[i];
				// 본 중복 확인, 본 이름과 노드 검증
				if (!_bones.contains(currentBone->mName.C_Str()))
				{
					BoneData bone;
					bone.name = mesh->mBones[i]->mName.C_Str();
					bone.id = _bones.size();
					bone.offsetTransform = ConvertToXMFLOAT4X4(mesh->mBones[i]->mOffsetMatrix);

					_bones.insert({ bone.name, bone });
				}
			}
		}
	}

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		ProcessNodes(node->mChildren[i], scene, currNode);
	}
}

void AssetLoader::MapWeights()
{
	for (auto& weight : _tempBoneWeights)
	{
		int subMeshIndex = weight.first.first;
		string boneName = weight.first.second;
		UINT boneId = _bones[boneName].id;
		shared_ptr<Mesh> mesh = _meshes[subMeshIndex];
		mesh->SetWeights(boneId, weight.second);
	}
}

void AssetLoader::MapBones()
{
	for (auto bone = _bones.begin(); bone != _bones.end(); bone++)
	{
		shared_ptr<NodeTempData> node = _nodes[bone->first];
		if (node == nullptr)
			continue;
		bone->second.node = node;
	}
}

void AssetLoader::BuildBones()
{
	vector<BoneData> sortedBones;
	sortedBones.reserve(_bones.size());
	for (auto& b : _bones)
		sortedBones.push_back(b.second);

	sort(sortedBones.begin(), sortedBones.end(), [](BoneData a, BoneData b) { return a.id < b.id; });

	for (auto& b : sortedBones)
	{
		shared_ptr<GameObject> foundObj = nullptr;
		shared_ptr<NodeTempData> currentParent = b.node->parent;
		
		while (true)
		{
			if (currentParent == nullptr)
				break;
			if (_bones.contains(currentParent->name))
			{
				foundObj = _bones[currentParent->name].instancedObj;
				_bones[b.name].parentId = _bones[currentParent->name].id;
				break;
			}
			currentParent = currentParent->parent;
		}

		shared_ptr<GameObject> boneObj = make_shared<GameObject>();
		boneObj->SetName(b.name);

		if (foundObj != nullptr)
			boneObj->GetTransform()->SetParent(foundObj->GetTransform());
		boneObj->GetTransform()->SetLocalMatrix(b.node->transform);
		_bones[b.name].localBindTransform = b.node->transform;

		_boneObjs.push_back(boneObj);
		_loadedObject.push_back(boneObj);
		_bones[b.name].instancedObj = boneObj;
	}

	for (auto& meshObj : _meshObjs)
	{
		auto renderer = meshObj->GetComponent<SkinnedMeshRenderer>();
		renderer->SetRootBone(_boneObjs[0]->GetTransform());
	}

	_boneObjs[0]->GetTransform()->SetParent(_loadedObject[0]->GetTransform());
	assert(_boneObjs.size() != 0);
}

shared_ptr<Mesh> AssetLoader::ProcessMesh(aiMesh* aimesh, const aiScene* scene)
{
	shared_ptr<Geometry> geometry = make_shared<Geometry>();

	vector<Vertex> vertices;
	vector<UINT32> indices;

	// Get Vertices
	vertices.reserve(aimesh->mNumVertices);
	for (UINT i = 0; i < aimesh->mNumVertices; i++)
	{
		Vertex v;
		v.Position = { aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z };
		if (aimesh->HasNormals())
			v.Normal = { aimesh->mNormals[i].x, aimesh->mNormals[i].y, aimesh->mNormals[i].z };
		if (aimesh->HasTangentsAndBitangents())
			v.Tangent = { aimesh->mTangents[i].x, aimesh->mTangents[i].y, aimesh->mTangents[i].z };
		
		for (int j = 0; j < aimesh->GetNumUVChannels(); j++)
		{
			switch (_modelType)
			{
			case ModelFormat::FBX:
				v.TexC = { (float)aimesh->mTextureCoords[j][i].x, (float)aimesh->mTextureCoords[j][i].y };
				break;
			case ModelFormat::GLTF:
				v.TexC = { (float)aimesh->mTextureCoords[j][i].x, 1.0f - (float)aimesh->mTextureCoords[j][i].y };
				break;
			}
		}

		vertices.push_back(v);
	}

	// Get Indices
	UINT numIndices = 0;
	for (UINT i = 0; i < aimesh->mNumFaces; i++)
	{
		numIndices += aimesh->mFaces[i].mNumIndices;
	}

	indices.reserve(numIndices);
	for (UINT i = 0; i < aimesh->mNumFaces; i++)
	{
		aiFace face = aimesh->mFaces[i];
		for (UINT j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// 본 가중치 여부 확인 후 추가
	if (aimesh->HasBones())
	{
		for (int i = 0; i < aimesh->mNumBones; i++)
		{
			// 현재 서브메시에 대한 본별 가중치 임시 저장
			aiBone* currentBone = aimesh->mBones[i];
			for (int j = 0; j < currentBone->mNumWeights; j++)
			{
				if (currentBone->mWeights[j].mWeight == 0)
					continue;

				BoneWeight weight;
				weight.vertexIndex = currentBone->mWeights[j].mVertexId;
				weight.weight = currentBone->mWeights[j].mWeight;
				_tempBoneWeights[{ _meshes.size(), currentBone->mName.C_Str() }].push_back(weight);
			}
		}
	}

	geometry->SetVertices(vertices);
	geometry->SetIndices(indices);

	shared_ptr<Mesh> mesh = make_shared<Mesh>(geometry);
	mesh->SetName(Utils::ToWString(aimesh->mName.C_Str()));
	auto mat = RESOURCE->Get<Material>(GetAIMaterialName(scene, aimesh->mMaterialIndex));
	mesh->SetMaterial(mat);
	return mesh;
}


// 애니메이션 데이터 추출
void AssetLoader::ProcessAnimation(const aiScene* scene)
{
	if (!scene->HasAnimations())
		return;

	LoadBones();
	_loadedObject[0]->AddComponent(ComponentFactory::Create("Animator"));

	// 애니메이션 갯수만큼
	for (int i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation* anim = scene->mAnimations[i];
		shared_ptr<Animation> bAnim = make_shared<Animation>(anim->mName.C_Str(), anim->mDuration, anim->mTicksPerSecond);

		for (int j = 0; j < anim->mNumChannels; j++)
		{
			aiNodeAnim* channel = anim->mChannels[j];
			string name = channel->mNodeName.data;

			Animation::AnimationData animData;
			animData.boneName = name;
			if (_bones.contains(name))
				animData.boneId = _bones[name].id;
			else
				animData.boneId = -1;

			UINT maxKeyCount = max({ channel->mNumPositionKeys, channel->mNumRotationKeys, channel->mNumScalingKeys });

			// position, rotation, scale 키프레임 받아오기
			map<float, Animation::KeyFrame> keyframeMap;
			for (int k = 0; k < maxKeyCount; k++)
			{
				if (k < channel->mNumPositionKeys)
				{
					aiVectorKey pos = channel->mPositionKeys[k];
					if (!keyframeMap.contains(pos.mTime))
						keyframeMap[pos.mTime].tick = pos.mTime;
					keyframeMap[pos.mTime].position = { pos.mValue.x, pos.mValue.y, pos.mValue.z };
				}
				
				if (k < channel->mNumRotationKeys)
				{
					aiQuatKey rot = channel->mRotationKeys[k];
					if (!keyframeMap.contains(rot.mTime))
						keyframeMap[rot.mTime].tick = rot.mTime;
					keyframeMap[rot.mTime].rotation = { rot.mValue.x, rot.mValue.y, rot.mValue.z, rot.mValue.w };
				}

				if (k < channel->mNumScalingKeys)
				{
					aiVectorKey scale = channel->mScalingKeys[k];
					if (!keyframeMap.contains(scale.mTime))
						keyframeMap[scale.mTime].tick = scale.mTime;
					keyframeMap[scale.mTime].scale = { scale.mValue.x, scale.mValue.y, scale.mValue.z };
				}
			}

			for (auto& kf : keyframeMap)
				animData.keyFrames.push_back(kf.second);

			bAnim->AddAnimationData(animData);
		}
		_animations.push_back(bAnim);
		_loadedObject[0]->GetComponent<Animator>()->AddAnimation(bAnim);
	}
}

void AssetLoader::LoadBones()
{
	if (_bones.size() == 0) {
		cout << "No bones found. Type .bbone file or blank." << endl;
		cout << "* Type blank will fill boneId to -1" << endl;
		cout << "bbone file name: ";
		string bboneName;
		getline(cin, bboneName);
		if (bboneName != "") {
			_bones = RESOURCE->LoadBone(bboneName);

			if (_animations.size() != 0) {
				for (auto& animation : _animations) {
					for (auto& animData : *animation->GetAnimationDatasPtr()) {
						if (_bones.contains(animData.boneName)) {
							animData.boneId = _bones[animData.boneName].id;
						}
						else {
							animData.boneId = -1;
						}
					}
				}
			}

			_isExternalBone = true;
		}
	}
}

wstring AssetLoader::GetAIMaterialName(const aiScene* scene, UINT index)
{
	string matNameStr(scene->mMaterials[index]->GetName().C_Str());
	return Utils::ToWString(matNameStr);
}

void AssetLoader::ImportModelFormat(wstring fileName)
{
	_assetNameW = fileName;
	_assetNameW.erase(_assetNameW.find_last_of(L"."), wstring::npos);
	_assetName = Utils::ToString(_assetNameW);

	istringstream ss(Utils::ToString(fileName));
	string format;
	while (getline(ss, format, '.'));

	if (format == "fbx")
		_modelType = ModelFormat::FBX;
	else if (format == "gltf")
		_modelType = ModelFormat::GLTF;
	else
		_modelType = ModelFormat::UNKOWN;
}
