#include "pch.h"
#include "ShadowMap.h"

ShadowMap::ShadowMap(UINT width, UINT height)
{
	_width = width;
	_height = height;
	_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, (float)width, (float)height);
	_scissorRect = CD3DX12_RECT(0, 0, (LONG)width, (LONG)height);

	BuildResource();
}

void ShadowMap::BuildDescriptors()
{
	_dsvHeapIndex = GRAPHIC->GetAndIncreaseDSVIndex();
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDsv(GRAPHIC->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart());
	hCpuDsv.Offset(_dsvHeapIndex, GRAPHIC->GetDSVDescriptorSize());
	
	// DSV
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	GRAPHIC->GetDevice()->CreateDepthStencilView(_shadowMap.Get(), &dsvDesc, hCpuDsv);
	_dsvHandle = hCpuDsv;

	// SRV
	_srvHeapIndex = GRAPHIC->GetAndIncreaseSRVHeapIndex();
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv(GRAPHIC->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart());
	hCpuSrv.Offset(_srvHeapIndex, GRAPHIC->GetCBVSRVDescriptorSize());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	GRAPHIC->GetDevice()->CreateShaderResourceView(_shadowMap.Get(), &srvDesc, hCpuSrv);

	_srvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		GRAPHIC->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart(),
		_srvHeapIndex,
		GRAPHIC->GetCBVSRVDescriptorSize());
}

void ShadowMap::BuildResource()
{
	D3D12_RESOURCE_DESC texDesc =
		CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, _width, _height);
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil = { 1.0f, 0 };

	ThrowIfFailed(GRAPHIC->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&clearValue,
		IID_PPV_ARGS(&_shadowMap)));
}
