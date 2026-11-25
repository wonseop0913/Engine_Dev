#include "pch.h"
#include "UISlider.h"

UISlider::UISlider() : UIElement(UIType::Slider)
{
	_name = "UISlider";

	_background = UI->CreateUI<UIPanel>();
	_fill = UI->CreateUI<UIPanel>();
}

UISlider::~UISlider()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - UISlider\n";
#endif

	_background.reset();
	_fill.reset();
}

void UISlider::Init()
{
	_transform->SetSize({ 200.0f, 20.0f });
	_transform->SetPivot({ 0.0f, 1.0f });

	_transformBg = _background->GetTransform();
	_transformBg->SetParent(_transform);
	_transformBg->SetSize(_transform->GetSize());
	_background->SetColor({ 0.7f, 0.7f, 0.7f, 1.0f });

	_transformFill = _fill->GetTransform();
	_transformFill->SetParent(_transform);
	_transformFill->SetPivot({ 0.0f, 0.5f });
	_transformFill->SetLocalPosition({ -100.0f, 0.0f, 0.0f });
	_transformFill->SetSize(_transform->GetSize());
}

void UISlider::Update()
{
	if (_isDirty) {
		if (_min > _max) _max = _min;
		if (_max < _min) _min = _max;
		_value = clamp(_value, _min, _max);

		_transformFill->SetWidth({ _transform->GetWidth() * _value / (_max - _min) });

		_isDirty = false;
	}
}

void UISlider::LoadXML(XMLElement* uiElem)
{
	throw std::logic_error("The method or operation is not implemented.");
}
