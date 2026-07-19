#pragma once

#define DEFAULT_INSTANCE_COUNT		16384		// 1024 * 16
#define	DEFAULT_MATERIAL_COUNT		256
#define DEFAULT_COUNT_LIGHT			20

#define BUFFER_COUNT	4

struct FrameResource
{
public:
	FrameResource();
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	void Init();
	void Update();

public:
	UINT GetInstanceSRVHeapIndex()const { return _instanceSrvHeapIndex; }
	UINT GetMaterialSRVHeapIndex()const { return _materialSrvHeapIndex; }
	UINT GetLightSRVHeapIndex()const { return _lightSrvHeapIndex; }

private:
	void UpdateObjectSB();
	void UpdateMaterialSB();
	void UpdateLightSB();

	void UpdateCameraCB();

	void BuildInstanceBufferSRV();
	void BuildMaterialBufferSRV();
	void BuildLightBufferSRV();

public:
	ID3D12CommandAllocator* cmdListAlloc[3];

	unique_ptr<UploadBuffer<InstanceConstants>> instanceSB = nullptr;
	unique_ptr<UploadBuffer<MaterialConstants>> materialSB = nullptr;
	unique_ptr<UploadBuffer<LightConstants>> lightSB = nullptr;

	unique_ptr<UploadBuffer<CameraConstants>> cameraCB = nullptr;

	UINT64 fence = 0;

private:
	UINT _instanceSrvHeapIndex = 0;
	UINT _materialSrvHeapIndex = 0;
	UINT _lightSrvHeapIndex = 0;

	bool _isInitialized = false;

	vector<int> _instanceIndices;

	// (Buffer Count) - (Material) <- Material Update Needs only few resources.
	array<future<void>, BUFFER_COUNT - 1> _futures;
};