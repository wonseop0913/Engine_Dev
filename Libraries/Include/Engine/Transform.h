#pragma once
#include "Component.h"

class BULB_API Transform : public Component
{
	using Super = Component;
public:
	Transform();
	virtual ~Transform();

#ifdef BULB_EDITOR
	virtual bool ShowComponentEditorGUI() override;
#endif

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;
	void SaveXML(Bulb::XMLElement compElem) override;

	shared_ptr<Component> Duplicate() override;

	ComponentSnapshot CaptureSnapshot() override;
	void RestoreSnapshot(ComponentSnapshot snapshot) override;

public:
	void ForceUpdateTransform();

	Bulb::Vector3 GetLocalPosition() { 
		if (_isDirty)
			UpdateTransform();

		return _localPosition; 
	}
	void SetLocalPosition(const Bulb::Vector3& position) { 
		_localPosition = position; 
		SetDirtyFlag();
	}
	
	// Get local rotation on degree
	Bulb::Vector3 GetLocalRotation() {
		if (_isDirty)
			UpdateTransform();

		return MathHelper::RadianToDegree(_localRotation);
	}

	// Set local rotation on degree
	void SetLocalRotation(const Bulb::Vector3& rotation) {
		SetLocalRotationRadian(MathHelper::DegreeToRadian(rotation));
	}

	// Get/Set Local Rotation With Radian
	Bulb::Vector3 GetLocalRotationRadian() {
		if (_isDirty)
			UpdateTransform();

		return _localRotation;
	}
	void SetLocalRotationRadian(const Bulb::Vector3& rotation);

	Bulb::Vector4 GetQuaternion() {
		if (_isDirty)
			UpdateTransform();

		return _quaternion;
	}
	void SetQuaternion(const Bulb::Vector4& quaternion);
	void SetQuaternion(const XMVECTOR& quaternion);

	Bulb::Vector4 GetLocalQuaternion() {
		if (_isDirty)
			UpdateTransform();

		return _localQuaternion;
	}
	void SetLocalQuaternion(const Bulb::Vector4& quaternion);
	void SetLocalQuaternion(const XMVECTOR& quaternion);

	Bulb::Vector3 GetLocalScale() {
		if (_isDirty)
			UpdateTransform();

		return _localScale;
	}
	void SetLocalScale(const Bulb::Vector3& scale) { 
		_localScale = scale; 
		SetDirtyFlag();
	}

	Bulb::Vector3 GetPosition() { 
		if (_isDirty)
			UpdateTransform();

		return _position; 
	}
	void SetPosition(const Bulb::Vector3& worldPosition);

	// Get rotation on degree
	Bulb::Vector3 GetRotation() {
		if (_isDirty)
			UpdateTransform();

		return MathHelper::RadianToDegree(_rotation); 
	}

	// Set rotation on degree
	void SetRotation(const Bulb::Vector3& worldRotation) { 
		SetRotationRadian(MathHelper::DegreeToRadian(worldRotation));
	}

	// Get/Set Rotation With Radian
	Bulb::Vector3 GetRotationRadian() {
		if (_isDirty)
			UpdateTransform(); 

		return _rotation;
	}
	void SetRotationRadian(const Bulb::Vector3& worldRotation);

	Bulb::Vector3 GetScale() {
		if (_isDirty)
			UpdateTransform();

		return _scale;
	}

	void SetScale(const Bulb::Vector3& worldScale);

	XMMATRIX GetRotationMatrix();

	Bulb::Vector3 GetRight();
	Bulb::Vector3 GetLeft();
	Bulb::Vector3 GetUp();
	Bulb::Vector3 GetDown();
	Bulb::Vector3 GetLook();
	Bulb::Vector3 GetBack();

	void Translate(const Bulb::Vector3& moveVec);

	void Rotate(const Bulb::Vector3& angle);
	void Rotate(const Bulb::Vector4& quat);
	void Rotate(const XMVECTOR& angle);

	void LookAt(const Bulb::Vector3& targetPos);
	void LookAtWithNoRoll(const Bulb::Vector3& targetPos, float blendAlpha = 1.0f);
	void LookAtOnlyYaw(const Bulb::Vector3& targetPos, float blendAlpha = 1.0f);

	void SetLocalMatrix(XMMATRIX mat);

	void SetLocalMatrix(XMFLOAT4X4 mat);
	void SetWorldMatrix(XMFLOAT4X4 mat);

	XMFLOAT4X4 GetLocalMatrix();
	XMFLOAT4X4 GetWorldMatrix();

	shared_ptr<Transform> GetParent() { return _parent; }
	void SetParent(shared_ptr<Transform> parent);

	// Set parent with out calculate local matrix
	void SetParentOnly(shared_ptr<Transform> parent);

	const vector<shared_ptr<Transform>>& GetChilds() { return _childs; }
	shared_ptr<Transform> GetChild(const string& name);
	bool RemoveChild(shared_ptr<Transform> child);

	bool HasParent() { return _parent != nullptr; }

	void SetDirtyFlag();
	bool IsDirty() { return _isDirty; }

	UINT GetDepthLevel() { return _depthLevel; }

protected:
	void AddChild(shared_ptr<Transform> child);
	void UpdateDepthLevel();

private:
	void UpdateTransform();

private:
	bool _isDirty = false;

	Bulb::Vector3 _localPosition;
	Bulb::Vector3 _localRotation;
	Bulb::Vector4 _localQuaternion;
	Bulb::Vector3 _localScale;

	Bulb::Vector3 _position;
	Bulb::Vector3 _rotation;
	Bulb::Vector4 _quaternion;
	Bulb::Vector3 _scale;

	XMFLOAT4X4 _matLocal;
	XMFLOAT4X4 _matWorld;

	shared_ptr<Transform> _parent;
	vector<shared_ptr<Transform>> _childs;

	UINT _depthLevel;
};

