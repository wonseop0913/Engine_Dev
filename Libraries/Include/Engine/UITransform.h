#pragma once

class UIElement;

class BULB_API UITransform : public enable_shared_from_this<UITransform>
{
	friend class UITransform;
public:
	UITransform(shared_ptr<UIElement> element);
	~UITransform();

public:
	void UpdateTransform();
	void OnResolutionUpdate();

	void SetPivot(const Bulb::Vector2& pivot);
	void SetPosition(const Bulb::Vector3& position);
	void SetLocalPosition(const Bulb::Vector3& position);
	void SetStretchSize(bool value) { _stretchByParent = value; }
	void SetSize(const Bulb::Vector2& size);
	void SetWidth(float width) {
		SetDirtyFlag();
		_size.x = width;
	}
	void SetHeight(float height) {
		SetDirtyFlag();
		_size.y = height;
	}
	void SetDepth(float depth);
	void SetDynamicPosition(bool value) { _isDynamicPosition = value; }

	Bulb::Vector2 GetPivot() { return _pivot; }
	Bulb::Vector3 GetPosition();
	Bulb::Vector3 GetLocalPosition() {
		if (_isDirty) UpdateTransform();
		return _localPosition; 
	}
	Bulb::Vector2 GetSize() { return _size; }
	float GetWidth() { return _size.x; }
	float GetHeight() { return _size.y; }
	float GetDepth() { 
		return _depth; 
	}
	bool IsDynamicPosition() { return _isDynamicPosition; }

	void SetParent(shared_ptr<UITransform> parent);
	void AddChild(shared_ptr<UITransform> child);

	const vector<shared_ptr<UITransform>>& GetChilds() { return _childs; }

	shared_ptr<UIElement> GetUIElement() { return _element.lock(); }

	bool IsDirty() { return _isDirty; }

	bool CheckInRect(float x, float y);

private:
	void SetDirtyFlag();

private:
	bool _isDirty = true;

	Bulb::Vector2 _pivot = { 0.5f, 0.5f };

	bool _isDynamicPosition = false;
	Bulb::Vector3 _position = { 0.0f, 0.0f, 0.0f };
	Bulb::Vector3 _localPosition = { 0.0f, 0.0f, 0.0f };
	bool _stretchByParent = false;
	Bulb::Vector2 _stretchPercentage = { 100.0f, 100.0f };
	Bulb::Vector2 _size = { 100.0f, 100.0f };
	float _depth = 1.0f;

	shared_ptr<UITransform> _parent = nullptr;
	vector<shared_ptr<UITransform>> _childs;

	weak_ptr<UIElement> _element;
};

