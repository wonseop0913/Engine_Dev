#pragma once
#include "UIElement.h"

class BULB_API UIText : public UIPanel
{
public:
	UIText();
	~UIText();

	void Init() override;
	void Update() override;

	void LoadXML(XMLElement* uiElem) override;

public:
	void SetText(const string& text) { SetText(Utils::ToWString(text)); }

	void SetText(const wstring& text);

	void SetFont(const string& fontName) { SetFont(Utils::ToWString(fontName)); }

	void SetFont(const wstring& fontName);

	void SetFontSize(float fontSize);
	void SetTextAlignment(DWRITE_TEXT_ALIGNMENT textAlignment) {
		_textAlignment = textAlignment;
		_textFormat->SetTextAlignment(_textAlignment);
		_isDirty = true;
	}
	void SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment) {
		_paragraphAlignment = paragraphAlignment;
		_textFormat->SetParagraphAlignment(_paragraphAlignment);
		_isDirty = true;
	}
	void SetTextColor(const Bulb::Color& color);

	void SetSize(const Bulb::Vector2 size) { 
		_transform->SetSize(size);
	}

private:
	void BuildDescriptors();
	void BuildResource();
	void RenderTextToTexture();

	void UpdateTextFormat();
	void UpdateTextLayout();
	void UpdateBrush();

private:
	wstring _text = L"Default Text";
	wstring _fontName = L"Malgun Gothic";
	float _fontSize = 24.0f;
	Bulb::Color _textColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	DWRITE_TEXT_ALIGNMENT _textAlignment = DWRITE_TEXT_ALIGNMENT_CENTER;
	DWRITE_PARAGRAPH_ALIGNMENT _paragraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;

	DWRITE_TEXT_METRICS _metrics;

	ComPtr<IDWriteTextLayout>	_textLayout;
	ComPtr<IDWriteTextFormat>	_textFormat;
	ComPtr<ID2D1SolidColorBrush> _brush;

	ComPtr<ID3D12Resource>		_textTexture;
	ComPtr<ID3D11Resource>		_wrappedResource;
	ComPtr<ID2D1Bitmap1>		_d2dBitmap;

	D3D12_GPU_DESCRIPTOR_HANDLE _srvHandle;

	UINT _textureWidth = 0;
	UINT _textureHeight = 0;
	bool _isTextureDirty = true;
	bool _isDirty = true;
};

