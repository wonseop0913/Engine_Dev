#pragma once
#include "XMLElement.h"

#pragma region 전방선언
class GameObject;
class Transform;
class Camera;
#pragma endregion

class BULB_API Component : public enable_shared_from_this<Component>
{
public:
	Component(ComponentType type);
	virtual ~Component();

	virtual void Init() { }
	virtual void PreUpdate() { }
	virtual void Update() { }
	virtual void Render(ID3D12GraphicsCommandList* cmdList, UINT renderState) { }

#ifdef BULB_EDITOR
	// Returns true when component should delete by GUI function.
	virtual bool ShowComponentEditorGUI() = 0;
#endif

	virtual void OnDestroy() = 0;

	virtual void OnCollisionEnter(shared_ptr<GameObject> other) { }
	virtual void OnCollision(shared_ptr<GameObject> other) { }
	virtual void OnCollisionExit(shared_ptr<GameObject> other) { }

	virtual void LoadXML(Bulb::XMLElement compElem) = 0;
	virtual void SaveXML(Bulb::XMLElement compElem) = 0;

	virtual shared_ptr<Component> Duplicate() = 0;

	virtual ComponentSnapshot CaptureSnapshot() = 0;
	virtual void RestoreSnapshot(ComponentSnapshot snapshot) = 0;

public:
	shared_ptr<GameObject> GetGameObject();
	shared_ptr<Transform> GetTransform();
	UINT GetID() { return _id; }

	bool IsActive() { return _isActive; }
	void SetActive(bool value) { if (type != ComponentType::Transform) _isActive = value; }

public:
	ComponentType type;
	bool isInitialized = false;

protected:
	UINT _id;
	weak_ptr<GameObject> _gameObject;
	bool _isActive = true;

private:
	friend class GameObject;
	void SetGameObject(shared_ptr<GameObject> gameObject) { _gameObject = gameObject; }

private:
	static UINT _idCount;

	bool _isSnapshotCaptured = false;
};

