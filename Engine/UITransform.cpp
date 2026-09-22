#include "pch.h"
#include "UITransform.h"

UITransform::UITransform(shared_ptr<UIElement> element)
{
	_element = element;
}

UITransform::~UITransform()
{
	cout << "Released - UITransform\n";
}

void UITransform::UpdateTransform()
{
	if (!_isDirty) return;

	CalcAnchoredLocalPosition();

	//if (_parent != nullptr) {
	//	_position = _parent->GetPosition() + _localPosition;
	//}
	//else {
	//	_position = _localPosition;
	//}

	_isDirty = false;
	for (auto& child : _childs) {
		child->SetDirtyFlag();
	}
}

void UITransform::OnResolutionUpdate()
{
	if (_stretchByParent)
		SetSize(_size);

	_isDirty = true;
}

void UITransform::SetPivot(const Bulb::Vector2& pivot)
{
	_pivot = pivot;

	float width = _size.x * (_pivot.x - 0.5f);
	float height = _size.y * (_pivot.y - 0.5f);

	_localPosition.x = _localPosition.x + width;
	_localPosition.y = _localPosition.y + height;
}

void UITransform::SetAnchor(UIAnchorMode anchorMode)
{
	if (anchorMode == _anchor) return;
	_anchor = anchorMode;

	CalcAnchoredPosition();
}

void UITransform::SetPosition(const Bulb::Vector3& position)
{
	_position = position;
	if (_parent != nullptr) {
		CalcAnchoredPosition();
		// _localPosition = _position - _parent->GetPosition();
	}
	else {
		_localPosition = _position;
	}

	SetDirtyFlag();
}

void UITransform::SetLocalPosition(const Bulb::Vector3& position)
{
	_localPosition = position;

	CalcAnchoredLocalPosition();
	//if (_parent != nullptr) {
	//	CalcAnchoredLocalPosition();
	//	// _position = _parent->GetPosition() + _localPosition;
	//}
	//else {
	//	_position = _localPosition;
	//}

	SetDirtyFlag();
}

void UITransform::SetSize(const Bulb::Vector2& size)
{
	SetDirtyFlag();
	if (_stretchByParent) {
		_stretchPercentage = size;

		Bulb::Vector2 parentSize;
		if (_parent != nullptr)
			parentSize = _parent->GetSize();
		else
			parentSize = { GRAPHIC->GetViewport().Width, GRAPHIC->GetViewport().Height };

		_size = { parentSize.x * _stretchPercentage.x, parentSize.y * _stretchPercentage.y };
	}
	else {
		_size = size;
	}
}

void UITransform::SetDepth(float depth)
{
	float depthDiff = depth - _depth;
	for (auto& child : _childs) {
		child->SetDepth(child->GetDepth() + depthDiff);
	}
	_depth = depth;

	UI->SetDepthSortFlag();
}

Bulb::Vector3 UITransform::GetPosition()
{
	if (_isDirty) UpdateTransform();
	float width = _size.x * (0.5f - _pivot.x);
	float height = _size.y * (0.5f - _pivot.y);

	if (_isDynamicPosition) {
		Bulb::Vector4 clipPos(XMVector3Transform(XMLoadFloat3(&_position), XMLoadFloat4x4(&Camera::GetViewProjMatrix())));
		Bulb::Vector2 ndc(clipPos.x / clipPos.w, clipPos.y / clipPos.w);
		D3D12_VIEWPORT viewport = GRAPHIC->GetViewport();
		return { ndc.x * 0.5f * viewport.Width, ndc.y * 0.5f * viewport.Height, _position.z };
	}
	else
		return { _position.x + width, _position.y + height, _position.z };
}

void UITransform::SetParent(shared_ptr<UITransform> parent)
{
	_parent = parent;

	if (_parent != nullptr) {
		UpdateTransform();
		_parent->AddChild(shared_from_this());
	}
}

void UITransform::AddChild(shared_ptr<UITransform> child)
{
	_childs.push_back(child);
}

bool UITransform::CheckInRect(float x, float y)
{
	float halfWidth = _size.x * 0.5f;
	float halfHeight = _size.y * 0.5f;

	return (x >= _position.x - halfWidth && x <= _position.x + halfWidth) && (y >= _position.y - halfHeight && y <= _position.y + halfHeight);
}

void UITransform::SetDirtyFlag()
{
	_isDirty = true;
	for (auto& child : _childs) {
		child->SetDirtyFlag();
	}
}

void UITransform::CalcAnchoredPosition()
{
	Bulb::Vector3 parentPos = _parent ? _parent->GetPosition() : Bulb::Vector3();
	Bulb::Vector2 parentHalfSize = (_parent ? _parent->GetSize() : Bulb::Vector2{ GRAPHIC->GetViewport().Width, GRAPHIC->GetViewport().Height }) / 2.0f;
	Bulb::Vector2 localPositionOffset(parentPos.x, parentPos.y);

	switch (_anchor) {
	case UIAnchorMode::LeftTop: {
		localPositionOffset.x -= parentHalfSize.x;
		localPositionOffset.y += parentHalfSize.y;
		break;
	}
	case UIAnchorMode::CenterTop: {
		localPositionOffset.y += parentHalfSize.y;
		break;
	}
	case UIAnchorMode::RightTop: {
		localPositionOffset.x += parentHalfSize.x;
		localPositionOffset.y += parentHalfSize.y;
		break;
	}
	case UIAnchorMode::LeftMid: {
		localPositionOffset.x -= parentHalfSize.x;
		break;
	}
	case UIAnchorMode::CenterMid: {
		// Nothing to calc
		break;
	}
	case UIAnchorMode::RightMid: {
		localPositionOffset.x += parentHalfSize.x;
		break;
	}
	case UIAnchorMode::LeftBot: {
		localPositionOffset.x -= parentHalfSize.x;
		localPositionOffset.y -= parentHalfSize.y;
		break;
	}
	case UIAnchorMode::CenterBot: {
		localPositionOffset.y -= parentHalfSize.y;
		break;
	}
	case UIAnchorMode::RightBot: {
		localPositionOffset.x += parentHalfSize.x;
		localPositionOffset.y -= parentHalfSize.y;
		break;
	}
	}

	_localPosition.x = _position.x - localPositionOffset.x;
	_localPosition.y = _position.y - localPositionOffset.y;
	_localPosition.z = _position.z - parentPos.z;
}

void UITransform::CalcAnchoredLocalPosition()
{
	Bulb::Vector3 parentPos = _parent ? _parent->GetPosition() : Bulb::Vector3();
	Bulb::Vector2 parentHalfSize = (_parent ? _parent->GetSize() : Bulb::Vector2{ GRAPHIC->GetViewport().Width, GRAPHIC->GetViewport().Height }) / 2.0f;
	Bulb::Vector2 localPositionOffset(parentPos.x, parentPos.y);

	switch (_anchor) {
	case UIAnchorMode::LeftTop: {
		localPositionOffset.x -= parentHalfSize.x;
		localPositionOffset.y += parentHalfSize.y;
		break;
	}
	case UIAnchorMode::CenterTop: {
		localPositionOffset.y += parentHalfSize.y;
		break;
	}
	case UIAnchorMode::RightTop: {
		localPositionOffset.x += parentHalfSize.x;
		localPositionOffset.y += parentHalfSize.y;
		break;
	}
	case UIAnchorMode::LeftMid: {
		localPositionOffset.x -= parentHalfSize.x;
		break;
	}
	case UIAnchorMode::CenterMid: {
		// Nothing to calc
		break;
	}
	case UIAnchorMode::RightMid: {
		localPositionOffset.x += parentHalfSize.x;
		break;
	}
	case UIAnchorMode::LeftBot: {
		localPositionOffset.x -= parentHalfSize.x;
		localPositionOffset.y -= parentHalfSize.y;
		break;
	}
	case UIAnchorMode::CenterBot: {
		localPositionOffset.y -= parentHalfSize.y;
		break;
	}
	case UIAnchorMode::RightBot: {
		localPositionOffset.x += parentHalfSize.x;
		localPositionOffset.y -= parentHalfSize.y;
		break;
	}
	}

	_position = Bulb::Vector3(
		localPositionOffset.x + _localPosition.x,
		localPositionOffset.y + _localPosition.y,
		parentPos.z + _localPosition.z);
}
