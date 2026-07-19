#include "pch.h"
#include "GameObject.h"

UINT32 GameObject::_preUpdateBypassFlag = 
	(UINT32)ComponentType::Rigidbody;

UINT32 GameObject::_updateBypassFlag = 
	(UINT32)ComponentType::ParticleEmitter | 
	(UINT32)ComponentType::Animator | 
	(UINT32)ComponentType::Light;

UINT32 GameObject::_renderBypassFlag = 
	(UINT32)ComponentType::ParticleEmitter |
	(UINT32)ComponentType::Terrain;

int GameObject::_idCount = 0;

GameObject::GameObject()
{
	primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	_id = _idCount++;

	_name = "GameObject";
	_psoName = PSO_OPAQUE_SOLID;
	_tag = "Default";

	_numFramesDirty = RENDER->GetNumFrameResources();
	_isInitialized = false;
}

GameObject::~GameObject()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - GameObject:" << _id << "\n";
#endif

	if (!_isDestroyed)
		OnDestroy();
}

void GameObject::Init()
{
	for (auto& componentVec : _components) {
		if (componentVec.size() == 0) continue;

		for (auto& c : componentVec) {
			c->isInitialized = true;
			c->Init();
		}
	}
}

void GameObject::PreUpdate()
{
	if (!_isActive || _parentInactiveStack > 0) return;

	if (!_isInitialized) {
		_isInitialized = true;
		Init();
	}

	if (_isDeleteReserved) {
		_deleteTime -= TIME->DeltaTime();
		if (_deleteTime <= 0.0f) {
			auto& childs = GetTransform()->GetChilds();
			RENDER->DeleteGameobject(shared_from_this());
			return;
		}
	}

	for (auto& componentVec : _components) {
		if (componentVec.size() == 0) continue;

		for (auto& c : componentVec) {
			if (!c->IsActive()) continue;

			// 런타임 중 추가되는 컴포넌트
			if (!c->isInitialized) {
				c->isInitialized = true;
				c->Init();
			}

#ifdef BULB_EDITOR
			if (!EDITOR->IsOnPlay() && _tag != "EditorCamera") continue;
#endif

			if ((UINT32)c->type & _preUpdateBypassFlag)
				continue;

			c->PreUpdate();
		}
	}
}

void GameObject::Update()
{
#ifdef BULB_EDITOR
	if (!EDITOR->IsOnPlay() && _tag != "EditorCamera") return;
#endif

	if (!_isActive || _parentInactiveStack > 0) return;

	for (auto& componentVec : _components) {
		if (componentVec.size() == 0) continue;

		for (auto& c : componentVec) {
			if (!c->IsActive()) continue;

			if ((UINT32)c->type & _updateBypassFlag)
				continue;
			c->Update();
		}
	}
}

void GameObject::Render(ID3D12GraphicsCommandList* cmdList, UINT renderState)
{
	// 이따금 람다식에서 오브젝트가 nullptr이 아님에도 함수가 호출된 이후 본 함수에서 nullptr 에러가 발생하는 경우가 있음.
	// 원인 불명이므로 임시조치
	if (this == nullptr) return;

	if (!_isActive || _parentInactiveStack > 0) return;

	for (auto& componentVec : _components) {
		if (componentVec.size() == 0) continue;

		for (auto& c : componentVec) {
			if (!c->IsActive()) continue;

			if ((UINT32)c->type & _renderBypassFlag)
				continue;
			c->Render(cmdList, renderState);
		}
	}
}

void GameObject::OnCollisionEnter(shared_ptr<GameObject> other)
{
	if (!_isActive || _parentInactiveStack > 0) return;

	for (auto& componentVec : _components) {
		if (componentVec.size() == 0) continue;

		for (auto& c : componentVec) {
			if (!c->IsActive()) continue;

			c->OnCollisionEnter(other);
		}
	}
}

void GameObject::OnCollision(shared_ptr<GameObject> other)
{
	if (!_isActive || _parentInactiveStack > 0) return;

	for (auto& componentVec : _components) {
		if (componentVec.size() == 0) continue;

		for (auto& c : componentVec) {
			if (!c->IsActive()) continue;

			c->OnCollision(other);
		}
	}
}

void GameObject::OnCollisionExit(shared_ptr<GameObject> other)
{
	if (!_isActive || _parentInactiveStack > 0) return;

	for (auto& componentVec : _components) {
		if (componentVec.size() == 0) continue;

		for (auto& c : componentVec) {
			if (!c->IsActive()) continue;

			c->OnCollisionExit(other);
		}
	}
}

void GameObject::OnDestroy()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "OnDestroy - GameObject:" << _id << "\n";
#endif

	auto childs = GetTransform()->GetChilds();
	for (auto& c : childs) {
		c->OnDestroy();
	}

	for (auto& componentVec : _components) {
		if (componentVec.size() == 0) continue;

		for (auto& c : componentVec) {
			if (c == nullptr) continue;		// Already released by RenderManager

			c->OnDestroy();
			c.reset();
			--_componentCount;
		}

		componentVec.clear();
	}

	_isDestroyed = true;
}

shared_ptr<GameObject> GameObject::Instantiate()
{
	shared_ptr<GameObject> go = make_shared<GameObject>();
	RENDER->AddGameObject(go);
	return go;
}

shared_ptr<GameObject> GameObject::LoadPrefab(string filePath)
{
	// shared_ptr<GameObject> go = RESOURCE->LoadPrefab(filePath)[0];
	shared_ptr<GameObject> go = RESOURCE->LoadPrefabXML(filePath);

	if (go != nullptr) {
		go->_isPrefab = true;
		go->_prefabPath = filePath;
	}
	return go;
}

shared_ptr<Component> GameObject::AddComponent(shared_ptr<Component> component)
{
	int componentTypeIdx = GetComponentTypeIndex(component->type);
	if (componentTypeIdx == -1) return nullptr;

	// Transform
	if (componentTypeIdx == 0) {
		if (_components[componentTypeIdx].size() > 0) {
			return _components[componentTypeIdx][0];
		}
	}
	
	++_componentCount;
	component->SetGameObject(shared_from_this());
	_components[componentTypeIdx].push_back(component);

	return component;
}

shared_ptr<Transform> GameObject::GetTransform()
{
	auto transform = GetComponent<Transform>();
	if (transform == nullptr) {
		transform = make_shared<Transform>();
		AddComponent(transform);
	}
	return transform;
}

void GameObject::DeleteComponent(shared_ptr<Component> component)
{
	if (component == nullptr) return;

	int componentTypeIdx = GetComponentTypeIndex(component->type);

	// Transform은 삭제되면 안됨
	if (componentTypeIdx == 0) return;

	--_componentCount;
	for (int i = 0; i < _components[componentTypeIdx].size(); ++i) {
		if (_components[componentTypeIdx][i] == component) {
			_components[componentTypeIdx].erase(_components[componentTypeIdx].begin() + i);
			component->OnDestroy();
		}
	}
}

int GameObject::GetComponentTypeIndex(ComponentType type)
{
	if (type == ComponentType::Undefined)			return -1;
	if (type == ComponentType::Transform)			return 0;
	if (type == ComponentType::MeshRenderer)		return 1;
	if (type == ComponentType::SkinnedMeshRenderer) return 2;
	if (type == ComponentType::Camera)				return 3;
	if (type == ComponentType::Script)				return 4;
	if (type == ComponentType::Light)				return 5;
	if (type == ComponentType::Animator)			return 6;
	if (type == ComponentType::Rigidbody)			return 7;
	if (type == ComponentType::ParticleEmitter)		return 8;
	if (type == ComponentType::CharacterController) return 9;
	if (type == ComponentType::Terrain)				return 10;
}

void GameObject::SetPSOName(const string& name)
{
	RENDER->UpdateObjectPSO(shared_from_this(), name);
	_psoName = name;
}

void GameObject::SetFramesDirty()
{
	// 컴포넌트 인스턴스 생성 후 아직 오브젝트 인스턴스에 추가하기 전 호출
	if (this != nullptr)
		_numFramesDirty = RENDER->GetNumFrameResources();
}

void GameObject::SetActive(bool flag)
{
	if (_isActive == flag) return;

	_isActive = flag;
	auto& childs = GetTransform()->GetChilds();
	if (_isActive) {
		for (auto& t : childs) {
			--t->GetGameObject()->_parentInactiveStack;
		}
	}
	else {
		for (auto& t : childs) {
			++t->GetGameObject()->_parentInactiveStack;
		}
	}
}

void GameObject::SetPrefabPath(string path)
{
	_prefabPath = path;
	_isPrefab = true;
}

shared_ptr<GameObject> GameObject::Duplicate()
{
	shared_ptr<GameObject> copy = GameObject::Instantiate();

	// 1. 컴포넌트 카피
	for (int i = 0; i < COUNT_COMPONENTTYPE; ++i) {
		if (_components[i].size() == 0) continue;

		for (int j = 0; j < _components[i].size(); ++j) {
			shared_ptr<Component> compCopy = _components[i][j]->Duplicate();
			copy->AddComponent(compCopy);
		}
	}

	// 2. 자식 오브젝트 재귀 카피
	auto childs = GetTransform()->GetChilds();
	for (auto& child : childs) {
		shared_ptr<GameObject> childCopy = child->GetGameObject()->Duplicate();
		childCopy->GetTransform()->SetParent(copy->GetTransform());
	}

	copy->SetName(_name);
	copy->SetPSOName(_psoName);
	if (_isPrefab) {
		copy->SetPrefabPath(_prefabPath);
	}

	copy->GetTransform()->SetParent(GetTransform()->GetParent());

	return copy;
}

void GameObject::Delete(float time)
{
	_isDeleteReserved = true;
	_deleteTime = time;
}

GameObjectSnapshot GameObject::CaptureSnapshot()
{
	_isSnapshotCaptured = true;

	GameObjectSnapshot snapshot;

	snapshot.id = _id;
	snapshot.name = _name;
	snapshot.pso = _psoName;

	return snapshot;
}

void GameObject::RestoreSnapshot(GameObjectSnapshot snapshot)
{
	_isSnapshotCaptured = false;
	SetName(snapshot.name);
	SetPSOName(snapshot.pso);
}
