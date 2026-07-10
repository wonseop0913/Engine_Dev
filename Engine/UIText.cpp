#include "pch.h"
#include "UIText.h"

UIText::UIText() : UIPanel(UIType::Text)
{

}

UIText::~UIText()
{
	cout << "Released - UIText\n";

	_textLayout.Reset();
	_textFormat.Reset();
	_brush.Reset();

	_textTexture.Reset();
	_wrappedResource.Reset();
	_d2dBitmap.Reset();
}

void UIText::Init()
{
	_textureSrvHeapIndex = GRAPHIC->GetAndIncreaseSRVHeapIndex();

	UpdateTextFormat();
	UpdateTextLayout();
	UpdateBrush();
}

void UIText::Update()
{
	if (_isDirty) {
		UpdateTextLayout();

		BuildResource();
		BuildDescriptors();

		RenderTextToTexture();

		_isDirty = false;
	}
}

void UIText::LoadXML(XMLElement* uiElem)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void UIText::SetText(const wstring& text)
{
	_isDirty = true;

	_text = text;
}

void UIText::SetFont(const wstring& fontName)
{
	_fontName = fontName;

	UpdateTextFormat();
}

void UIText::SetFontSize(float fontSize)
{
	_fontSize = fontSize;

	UpdateTextFormat();
}

void UIText::SetTextColor(const Bulb::Color& color)
{
	_textColor = color;

	UpdateBrush();
}

void UIText::BuildDescriptors()
{
	// SRV
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv(GRAPHIC->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart());
	hCpuSrv.Offset(_textureSrvHeapIndex, GRAPHIC->GetCBVSRVDescriptorSize());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	GRAPHIC->GetDevice()->CreateShaderResourceView(_textTexture.Get(), &srvDesc, hCpuSrv);

	_srvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		GRAPHIC->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart(),
		_textureSrvHeapIndex,
		GRAPHIC->GetCBVSRVDescriptorSize());
}

void UIText::BuildResource()
{
	D3D12_RESOURCE_DESC texDesc =
		CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_B8G8R8A8_UNORM, _transform->GetWidth(), _transform->GetHeight());
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	texDesc.MipLevels = 1;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	clearValue.Color[0] = 0.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 0.0f;

	ThrowIfFailed(GRAPHIC->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&clearValue,
		IID_PPV_ARGS(&_textTexture)));

	GRAPHIC->CreateWrappedResource(_textTexture.Get(), _wrappedResource.GetAddressOf());

	ComPtr<IDXGISurface> surface;
	ThrowIfFailed(_wrappedResource.As(&surface));

	ThrowIfFailed(GRAPHIC->GetDeviceContextD2D()->CreateBitmapFromDxgiSurface(
		surface.Get(),
		nullptr,
		&_d2dBitmap
	));
}

void UIText::RenderTextToTexture()
{
	GRAPHIC->GetDevice11On12()->AcquireWrappedResources(_wrappedResource.GetAddressOf(), 1);

	auto d2dContext = GRAPHIC->GetDeviceContextD2D();

	d2dContext->SetTarget(_d2dBitmap.Get());
	d2dContext->BeginDraw();

	d2dContext->Clear(D2D1::ColorF(0, 0, 0, 0.0f));

	if (_textLayout != nullptr)
	{
		d2dContext->DrawTextLayout(
			D2D1::Point2F(0.f, 0.f),
			_textLayout.Get(),
			_brush.Get()
		);
	}

	HRESULT hr = d2dContext->EndDraw();

	GRAPHIC->GetDevice11On12()->ReleaseWrappedResources(_wrappedResource.GetAddressOf(), 1);

	GRAPHIC->GetDeviceContextD11()->Flush();
}

void UIText::UpdateTextFormat()
{
	ThrowIfFailed(GRAPHIC->GetWriteFactory()->CreateTextFormat(
		_fontName.c_str(), nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		_fontSize, L"ko-kr", &_textFormat));

	_textFormat->SetTextAlignment(_textAlignment);
	_textFormat->SetParagraphAlignment(_paragraphAlignment);
}

void UIText::UpdateTextLayout()
{
	// Using transform size for size limit
	ThrowIfFailed(GRAPHIC->GetWriteFactory()->CreateTextLayout(
		_text.c_str(), _text.length(),
		_textFormat.Get(), _transform->GetWidth(), _transform->GetHeight(), &_textLayout));

	_textLayout->GetMetrics(&_metrics);
}

void UIText::UpdateBrush()
{
	ThrowIfFailed(GRAPHIC->GetDeviceContextD2D()->CreateSolidColorBrush(
		D2D1::ColorF(_textColor.r, _textColor.g, _textColor.b, _textColor.a), &_brush));
}
