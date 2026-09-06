#pragma once
#include "UIElement.h"

class BULB_API UISlider : public UIElement
{
	friend class UIManager;
public:
	UISlider();
	~UISlider();

	void Init() override;
	void Update() override;

	void LoadXML(XMLElement* uiElem) override;

public:
	void SetEntireSize(Bulb::Vector2 size) {
		_transform->SetSize(size);
		_transformBg->SetSize(size);
		_transformFill->SetSize(size);
		_transformFill->SetLocalPosition({ -size.x / 2.0f, 0.0f, 0.0f });
	}

	void SetValue(float value) {
		_value = value;
		_isDirty = true;
	}

	void SetValueMinLimit(float value) {
		_min = value;
		_isDirty = true;
	}

	void SetValueMaxLimit(float value) {
		_max = value;
		_isDirty = true;
	}

	void SetBackgroundColor(const Bulb::Color color) { _background->SetColor(color); }
	void SetFillColor(const Bulb::Color color) { _fill->SetColor(color); }

private:
	shared_ptr<UIPanel> _background;
	shared_ptr<UIPanel> _fill;

	shared_ptr<UITransform> _transformBg;
	shared_ptr<UITransform> _transformFill;

	bool _isDirty = true;

	float _value = 5.0f;
	float _min = 0.0f;
	float _max = 10.0f;
};

