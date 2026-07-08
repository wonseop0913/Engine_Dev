#pragma once

#define		DEFUALT_NUM_DESCRIPTORS		3

class Graphic
{
	friend class BulbApplication;
	friend class RenderManager;

private:
	Graphic() = default;
	~Graphic();

	bool Init();
	void Update();
	void LateUpdate();
	void RenderEnd(FrameResource* currentFrameResource);

	void OnResize();

public:
	Graphic(const Graphic& rhs) = delete;
	Graphic& operator=(const Graphic& rhs) = delete;

	static Graphic* GetInstance();
	static Bulb::ProcessResult Delete();

// Getter/Setter
public:
	HWND GetMainWnd() const { return _hMainWnd; }
	ComPtr<IDXGIFactory4> GetDXGIFactory()const { return _dxgiFactory; }

	float GetAspectRatio() const { return static_cast<float>(_appDesc.clientWidth) / _appDesc.clientHeight; }

	bool IsMSAAEnabled() const { return _appDesc._4xMsaaState; }
	void Set4xMsaaState(bool value) {
		if (_appDesc._4xMsaaState != value)
		{
			_appDesc._4xMsaaState = value;

			BuildSwapChain();
			OnResize();
		}
	}

	AppDesc GetAppDesc() const { return _appDesc; }
	void SetAppDesc(AppDesc appDesc) { _appDesc = appDesc; }

	ID3D12Device* GetDevice() const { return _device.Get(); }
	ID3D11Device* GetDevice11() const { return _d3d11Device.Get(); }
	ID3D11On12Device* GetDevice11On12() const { return _d3d11On12Device.Get(); }

	ID3D11DeviceContext* GetDeviceContextD11() const { return _d3d11DeviceContext.Get(); }
	ID2D1DeviceContext2* GetDeviceContextD2D() const { return _d2dContext.Get(); }

	ID3D12GraphicsCommandList* GetCommandList() const { return _graphicsCmdList; }
	ID3D12CommandQueue* GetCommandQueue() const { return _commandQueue.Get(); }
	IDXGISwapChain* GetSwapChain() const { return _swapChain.Get(); }
	IDWriteFactory* GetWriteFactory() const { return _dWriteFactory.Get(); }

	ComPtr<ID3D12DescriptorHeap> GetConstantBufferHeap()const { return _cbvHeap; }

	ComPtr<ID3D12DescriptorHeap> GetRTVHeap()const { return _rtvHeap; }
	UINT GetRTVDescriptorSize()const { return _rtvDescriptorSize; }

	ID3D12Resource* GetCurrentBackBuffer()const { return _swapChainBuffer[_currBackBuffer].Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView()const {
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
			_currBackBuffer,
			_rtvDescriptorSize);
	}

	ID3D12Resource* GetMSAARenderTarget() const { return _msaaRenderTarget.Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetMSAARTVHandle() const { return _msaaRtvHandle; }

	UINT GetAndIncreaseDSVHeapIndex() {
		return _dsvHeapIndex++;
	}
	ComPtr<ID3D12DescriptorHeap> GetDSVHeap()const { return _dsvHeap; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle()const { return _dsvHandle; }

	D3D12_VIEWPORT GetViewport()const { return _screenViewport; }
	D3D12_RECT GetScissorRect()const { return _scissorRect; }
	Bulb::Vector2 GetScreenCenter()const { return _center; }

	UINT GetDSVDescriptorSize()const { return _dsvDescriptorSize; }
	UINT GetCBVSRVDescriptorSize()const { return _cbvSrvUavDescriptorSize; }

	DXGI_FORMAT GetBackBufferFormat()const { return _backBufferFormat; }
	DXGI_FORMAT GetDepthStencilFormat()const { return _depthStencilFormat; }

public:
	void WaitForFence(UINT64 fenceValue);
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void CreateWrappedResource(ID3D12Resource* d12Res, ID3D11Resource** d11ResOut);

	void UseGraphicsCommandList();
	void ExecuteGraphicsCommandList();

private:
	bool InitMainWindow();
	bool InitDirect3D();
	void Init11On12();

	void BuildCommandObjects();
	void BuildSwapChain();
	void BuildDescriptorHeaps();
	void BuildMSAARenderTarget();

	void FlushCommandQueue();

public:
	int cursorMovePendingX = 0, cursorMovePendingY = 0;

private:
	static Graphic* s_instance;

	HWND      _hMainWnd = nullptr;

	// DX12
	ComPtr<ID3D12Device>			_device;

	// DX11
	ComPtr<ID3D11Device>			_d3d11Device;
	ComPtr<ID3D11DeviceContext>		_d3d11DeviceContext;
	ComPtr<ID3D11On12Device>		_d3d11On12Device;

	// DX2D
	ComPtr<ID2D1Factory3>			_d2dFactory;
	ComPtr<ID2D1Device2>			_d2dDevice;
	ComPtr<ID2D1DeviceContext2>		_d2dContext;

	// DirectWrite
	ComPtr<IDWriteFactory>			_dWriteFactory;

	ComPtr<IDXGIFactory4>			_dxgiFactory;
	ComPtr<IDXGISwapChain>			_swapChain;

	ComPtr<ID3D12Fence>				_fence;
	UINT64							_currentFence = 0;

	ComPtr<ID3D12CommandQueue>		_commandQueue;
	ID3D12GraphicsCommandList*		_graphicsCmdList;
	ID3D12CommandAllocator*			_graphicsCmdListAlloc;
	bool							_isCmdListUsed = false;

	static const int				_SwapChainBufferCount = 2;
	int								_currBackBuffer = 0;
	ComPtr<ID3D12Resource>			_swapChainBuffer[_SwapChainBufferCount];
	ComPtr<ID3D12Resource>			_depthStencilBuffer;

	// MSAA
	ComPtr<ID3D12Resource>			_msaaRenderTarget;
	D3D12_CPU_DESCRIPTOR_HANDLE		_msaaRtvHandle;
	UINT							_msaaSampleCount = 4;
	UINT							_msaaQuality = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE		_dsvHandle;
	UINT							_dsvHeapIndex = 0;

	ComPtr<ID3D12DescriptorHeap>	_rtvHeap;
	ComPtr<ID3D12DescriptorHeap>	_msaaRtvHeap;
	ComPtr<ID3D12DescriptorHeap>	_dsvHeap;
	ComPtr<ID3D12DescriptorHeap>	_cbvHeap;

	D3D12_VIEWPORT					_screenViewport;
	D3D12_RECT						_scissorRect;
	Bulb::Vector2					_center;

	UINT							_rtvDescriptorSize = 0;
	UINT							_dsvDescriptorSize = 0;
	UINT							_cbvSrvUavDescriptorSize = 0;

	D3D_DRIVER_TYPE					_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT						_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT						_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT						_msaaFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	AppDesc _appDesc;

	//===========================리팩토링 필수!!!!!!!!============

	UINT _passCbvOffset = 0;

	UINT _objCBByteSize = DXUtil::CalcConstantBufferByteSize(sizeof(InstanceConstants));
};

