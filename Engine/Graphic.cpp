#include "pch.h"
#include "Graphic.h"
#include "Camera.h"

Graphic* Graphic::s_instance = nullptr;

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return GRAPHIC->MsgProc(hwnd, msg, wParam, lParam);
}

Graphic::~Graphic()
{
	if (_device != nullptr)
		FlushCommandQueue();
}


bool Graphic::Init()
{
	if (!InitMainWindow())
		return false;

	if (!InitDirect3D())
		return false;

	GetAndIncreaseDSVIndex();
	AssignmentRTVHeapIndices();
	_mainSrvHeapIndex = GetAndIncreaseSRVHeapIndex();
	OnResize();

	UseGraphicsCommandList();
	ExecuteGraphicsCommandList();

	return true;
}


void Graphic::Update()
{
	INPUTM->OnMouseMove(cursorMovePendingX, cursorMovePendingY);
	cursorMovePendingX = 0;
	cursorMovePendingY = 0;
}


void Graphic::LateUpdate()
{
	ExecuteGraphicsCommandList();
}

Graphic* Graphic::GetInstance()
{
	if (s_instance == nullptr)
		s_instance = new Graphic();
	return s_instance;
}

Bulb::ProcessResult Graphic::Delete()
{
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
		return Bulb::ProcessResult::SUCCESS;
	}
	return Bulb::ProcessResult::FAILED_INSTANCE_NOT_FOUND;
}

void Graphic::WaitForFence(UINT64 fenceValue)
{
	if (fenceValue != 0 && _fence->GetCompletedValue() < fenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(_fence->SetEventOnCompletion(fenceValue, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void Graphic::OnResize()
{
	assert(_device);
	assert(_swapChain);

	FlushCommandQueue();

	ThrowIfFailed(_graphicsCmdListAlloc->Reset());
	ThrowIfFailed(_graphicsCmdList->Reset(_graphicsCmdListAlloc, nullptr));

	for (int i = 0; i < _SwapChainBufferCount; ++i)
		_swapChainBuffer[i].Reset();
	_depthStencilBuffer.Reset();

	ThrowIfFailed(_swapChain->ResizeBuffers(
		_SwapChainBufferCount,
		_appDesc.clientWidth, _appDesc.clientHeight,
		_backBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	_currBackBuffer = 0;

	for (UINT i = 0; i < _SwapChainBufferCount; i++) {
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(
			_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
			i,
			_rtvDescriptorSize);
		ThrowIfFailed(_swapChain->GetBuffer(i, IID_PPV_ARGS(&_swapChainBuffer[i])));
		_device->CreateRenderTargetView(_swapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
	}

	BuildMainPassRTV();
	BuildMainPassSRV();
	BuildMSAARTV();

	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = _appDesc.clientWidth;
	depthStencilDesc.Height = _appDesc.clientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	depthStencilDesc.Format = _depthStencilFormat;

	depthStencilDesc.SampleDesc.Count = _appDesc._4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = _appDesc._4xMsaaState ? (_appDesc._4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = _depthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(_depthStencilBuffer.GetAddressOf())));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = _appDesc._4xMsaaState ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = _depthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDsv(_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	_device->CreateDepthStencilView(_depthStencilBuffer.Get(), &dsvDesc, hDsv);
	_dsvHandle = hDsv;

	_graphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_depthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	ThrowIfFailed(_graphicsCmdList->Close());
	ID3D12CommandList* cmdsLists[] = { _graphicsCmdList };
	_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();

	_screenViewport.TopLeftX = 0;
	_screenViewport.TopLeftY = 0;
	_screenViewport.Width = static_cast<float>(_appDesc.clientWidth);
	_screenViewport.Height = static_cast<float>(_appDesc.clientHeight);
	_screenViewport.MinDepth = 0.0f;
	_screenViewport.MaxDepth = 1.0f;

	_scissorRect = { 0, 0, _appDesc.clientWidth, _appDesc.clientHeight };
	_center = { _appDesc.clientWidth / 2.0f, _appDesc.clientHeight / 2.0f };

	ENGINESTAT->ResetValues();
}


void Graphic::RenderEnd(FrameResource* currentFrameResource)
{
	_currBackBuffer = (_currBackBuffer + 1) % _SwapChainBufferCount;

	currentFrameResource->fence = ++_currentFence;

	_commandQueue->Signal(_fence.Get(), _currentFence);
}


// 윈도우 메세지 처리부
LRESULT Graphic::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// ImGUI test
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	AppStatus appStatus = APP->GetAppStatus();
	switch (msg)
	{
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				appStatus.appPaused = true;
				TIME->Stop();
			}
			else
			{
				appStatus.appPaused = false;
				TIME->Start();
			}
			APP->SetAppStatus(appStatus);
			return 0;

		case WM_SIZE:
			_appDesc.clientWidth = LOWORD(lParam);
			_appDesc.clientHeight = HIWORD(lParam);
			if (_device)
			{
				if (wParam == SIZE_MINIMIZED)
				{
					appStatus.appPaused = true;
					appStatus.minimized = true;
					appStatus.maximized = false;
				}
				else if (wParam == SIZE_MAXIMIZED)
				{
					appStatus.appPaused = false;
					appStatus.minimized = false;
					appStatus.maximized = true;
					OnResize();
				}
				else if (wParam == SIZE_RESTORED)
				{
					if (appStatus.minimized)
					{
						appStatus.appPaused = false;
						appStatus.minimized = false;
						OnResize();
					}

					else if (appStatus.maximized)
					{
						appStatus.appPaused = false;
						appStatus.maximized = false;
						OnResize();
					}
					else if (appStatus.resizing)
					{
						OnResize();
					}
				}
			}
			APP->SetAppStatus(appStatus);
			return 0;

		case WM_ENTERSIZEMOVE:
			appStatus.appPaused = true;
			appStatus.resizing = true;
			TIME->Stop();
			APP->SetAppStatus(appStatus);
			return 0;

		case WM_EXITSIZEMOVE:
			appStatus.appPaused = false;
			appStatus.resizing = false;
			TIME->Start();
			APP->SetAppStatus(appStatus);
			OnResize();
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_MENUCHAR:
			return MAKELRESULT(0, MNC_CLOSE);

		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
			return 0;

		case WM_INPUT:
		{
			static BYTE lpb[sizeof(RAWINPUT)];
			UINT dwSize = sizeof(RAWINPUT);

			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
				break;

			RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(lpb);

			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				cursorMovePendingX += raw->data.mouse.lLastX;
				cursorMovePendingY += raw->data.mouse.lLastY;
				//if (raw->data.mouse.lLastX != 0 || raw->data.mouse.lLastY != 0)
				//	INPUTM->OnMouseMove(raw->data.mouse.lLastX, raw->data.mouse.lLastY);

				if (raw->data.mouse.usButtonFlags != 0)
					INPUTM->OnMouseClick(raw->data.mouse.usButtonFlags);
			}

			return 0;
		}

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			return 0;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			return 0;
		case WM_MOUSEMOVE:
			//INPUTM->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_KEYUP:
			return 0;
		}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


void Graphic::CreateWrappedResource(ID3D12Resource* d12Res, ID3D11Resource** d11ResOut)
{
	D3D11_RESOURCE_FLAGS d11Flags = { D3D11_BIND_RENDER_TARGET };
	ThrowIfFailed(_d3d11On12Device->CreateWrappedResource(
		d12Res, &d11Flags,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		IID_PPV_ARGS(d11ResOut)
	));
}

void Graphic::UseGraphicsCommandList()
{
	if (_isCmdListUsed) return;

	_isCmdListUsed = true;

	ThrowIfFailed(_graphicsCmdListAlloc->Reset());
	ThrowIfFailed(_graphicsCmdList->Reset(_graphicsCmdListAlloc, nullptr));
}

void Graphic::ExecuteGraphicsCommandList()
{
	if (_isCmdListUsed) {
		_isCmdListUsed = false;

		ThrowIfFailed(_graphicsCmdList->Close());
		ID3D12CommandList* cmdsLists[] = { _graphicsCmdList };
		_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		FlushCommandQueue();
	}
}

// 윈도우 초기화
bool Graphic::InitMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = APP->GetAppInst();
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	RECT R = { 0, 0, _appDesc.clientWidth, _appDesc.clientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	_hMainWnd = CreateWindow(L"MainWnd", _appDesc.mainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, APP->GetAppInst(), 0);
	if (!_hMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(_hMainWnd, SW_SHOW);
	UpdateWindow(_hMainWnd);

	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01;
	rid.usUsage = 0x02;
	rid.dwFlags = RIDEV_INPUTSINK;
	rid.hwndTarget = _hMainWnd;

	RegisterRawInputDevices(&rid, 1, sizeof(rid));

	return true;
}


// dx 초기화
bool Graphic::InitDirect3D()
{
	#if defined(DEBUG) || defined(_DEBUG) 
		{
			ComPtr<ID3D12Debug> debugController;
			ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
			debugController->EnableDebugLayer();
		}
	#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)));

	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&_device));

	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&_device)));
	}

	ThrowIfFailed(_device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&_fence)));

	_rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_dsvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	_cbvSrvUavDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = _backBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(_device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	_appDesc._4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(_appDesc._4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
	DXUtil::LogAdapters();
#endif

	BuildCommandObjects();
	BuildSwapChain();
	BuildDescriptorHeaps();

	Init11On12();

	return true;
}

void Graphic::Init11On12()
{
	UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT; 
#if defined(DEBUG) || defined(_DEBUG)
		d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hardwareResult = D3D11On12CreateDevice(
		_device.Get(),
		d3d11DeviceFlags,
		nullptr,
		0,
		reinterpret_cast<IUnknown**>(_commandQueue.GetAddressOf()),
		1,
		0,
		&_d3d11Device,
		&_d3d11DeviceContext,
		nullptr
	);

	ThrowIfFailed(_d3d11Device.As(&_d3d11On12Device));

	// Direct2D 팩토리 생성
	D2D1_FACTORY_OPTIONS options = {};
#if defined(DEBUG) || defined(_DEBUG)
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
	ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &options, &_d2dFactory));

	//  DXGI 디바이스 획득 (D3D11 디바이스로부터)
	ComPtr<IDXGIDevice> dxgiDevice;
	ThrowIfFailed(_d3d11On12Device.As(&dxgiDevice));

	// D2D 디바이스 및 컨텍스트 생성
	ThrowIfFailed(_d2dFactory->CreateDevice(dxgiDevice.Get(), &_d2dDevice));
	ThrowIfFailed(_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &_d2dContext));

	// DirectWrite 팩토리 생성
	ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &_dWriteFactory));
}

void Graphic::AssignmentRTVHeapIndices()
{
	for (int i = 0; i < _SwapChainBufferCount; ++i)
		GetAndIncreaseRTVIndex();

	_mainRtvHeapIndex = GetAndIncreaseRTVIndex();
	_msaaRtvHeapIndex = GetAndIncreaseRTVIndex();
}

void Graphic::BuildCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue)));

	ThrowIfFailed(_device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&_graphicsCmdListAlloc)));

	ThrowIfFailed(_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		_graphicsCmdListAlloc,
		nullptr,
		IID_PPV_ARGS(&_graphicsCmdList)));

	_graphicsCmdList->Close();
}

void Graphic::BuildSwapChain()
{
	_swapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = _appDesc.clientWidth;
	sd.BufferDesc.Height = _appDesc.clientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = _backBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//sd.SampleDesc.Count = _appDesc._4xMsaaState ? 4 : 1;
	//sd.SampleDesc.Quality = _appDesc._4xMsaaState ? (_appDesc._4xMsaaQuality - 1) : 0;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = _SwapChainBufferCount;
	sd.OutputWindow = _hMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ThrowIfFailed(_dxgiFactory->CreateSwapChain(
		_commandQueue.Get(),
		&sd,
		_swapChain.GetAddressOf()));
}

void Graphic::BuildDescriptorHeaps()
{
	// Render Target View
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	// Current Usage: SwapChain(2) + MSAA(1) + MainPass(1) + EditorOutline(1)
	rtvHeapDesc.NumDescriptors = DEFAULT_NUM_RTVDESCRIPTORS;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(_device->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(_rtvHeap.GetAddressOf())));

	// Depth Stencil View
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = DEFUALT_NUM_DSVDESCRIPTORS;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(_device->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(_dsvHeap.GetAddressOf())));

	// Shader Resource View
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = DEFAULT_NUM_SRVDESCRIPTORS;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(GRAPHIC->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&_srvHeap)));
}

void Graphic::BuildMainPassRTV()
{
	D3D12_RESOURCE_DESC mainRTDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		_backBufferFormat,
		_appDesc.clientWidth,
		_appDesc.clientHeight
	);

	mainRTDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearVal = {};
	clearVal.Format = _backBufferFormat;
	clearVal.Color[3] = 1.0f;

	CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(_device->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE,
		&mainRTDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearVal, IID_PPV_ARGS(_mainRenderTarget.GetAddressOf())));

	_mainRtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		_mainRtvHeapIndex,
		_rtvDescriptorSize);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = _backBufferFormat;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	_device->CreateRenderTargetView(_mainRenderTarget.Get(), &rtvDesc, _mainRtvHandle);
}

void Graphic::BuildMSAARTV()
{
	if (!_appDesc._4xMsaaState) return;

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;
	msaaQualityLevels.Format = _backBufferFormat;
	msaaQualityLevels.SampleCount = _msaaSampleCount;
	msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msaaQualityLevels.NumQualityLevels = 0;

	_device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msaaQualityLevels,
		sizeof(msaaQualityLevels));

	// NumQualityLevels가 0이면 지원 안 함
	_msaaQuality = msaaQualityLevels.NumQualityLevels - 1;

	D3D12_RESOURCE_DESC msaaRTDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		_backBufferFormat,
		_appDesc.clientWidth,
		_appDesc.clientHeight,
		1,
		1,
		_msaaSampleCount
	);

	msaaRTDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearVal = {};
	clearVal.Format = _backBufferFormat;
	clearVal.Color[3] = 1.0f;

	CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(_device->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE,
		&msaaRTDesc, D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		&clearVal, IID_PPV_ARGS(_msaaRenderTarget.GetAddressOf())));

	_msaaRtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		_msaaRtvHeapIndex,
		_rtvDescriptorSize);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = _backBufferFormat;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

	_device->CreateRenderTargetView(_msaaRenderTarget.Get(), &rtvDesc, _msaaRtvHandle);
}

void Graphic::BuildMainPassSRV()
{
	// Main Pass SRV
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv(_srvHeap->GetCPUDescriptorHandleForHeapStart());
	hCpuSrv.Offset(_mainSrvHeapIndex, _cbvSrvUavDescriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = _backBufferFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	_device->CreateShaderResourceView(_mainRenderTarget.Get(), &srvDesc, hCpuSrv);

	_mainSrvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		_srvHeap->GetGPUDescriptorHandleForHeapStart(),
		_mainSrvHeapIndex,
		_cbvSrvUavDescriptorSize);
}

void Graphic::FlushCommandQueue()
{
	_currentFence++;

	ThrowIfFailed(_commandQueue->Signal(_fence.Get(), _currentFence));

	if (_fence->GetCompletedValue() < _currentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		ThrowIfFailed(_fence->SetEventOnCompletion(_currentFence, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}
