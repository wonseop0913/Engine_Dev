#pragma once

class BULB_API EditorManager
{
	friend class BulbApplication;

public:
	EditorManager() = default;
	~EditorManager();

	void Init();
	void Render(ID3D12GraphicsCommandList* cmdList);

public:
	EditorManager(const EditorManager& rhs) = delete;
	EditorManager& operator=(const EditorManager& rhs) = delete;

	static EditorManager* GetInstance();
	static Bulb::ProcessResult Delete();

	void Play();
	void Stop();

	bool IsOnPlay() { return _isOnPlay; }

	void SetEditorWindowText(string text);

	void LoadMeshes();
	void LoadPrefabs();

	shared_ptr<EditorCamera> GetEditorCamera() { return _editorCamera; }

	const vector<string>& GetPrefabList() { return _prefabDirectories; }

	ID3D12Resource* GetOutlineRenderTarget() { return _outlineRenderTarget.Get(); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return _rtvHandle; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetDsv() const { return _dsvHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrv() const { return _srvHandle; }

private:
	void BuildDescriptors();
	void BuildResource();
	void RestoreObjectComponents(shared_ptr<GameObject> go, GameObjectSnapshot objectSnapshot);

public:
	bool isNoneMouseMode = false;

private:
	static EditorManager* s_instance;

	bool _isOnPlay = false;
	string _currentWindowText;

	shared_ptr<EditorCamera> _editorCamera;

	vector<GameObjectSnapshot> _objectSnapshots;
	vector<ComponentSnapshot> _compSnapshots;

	vector<string> _prefabDirectories;

	ComPtr<ID3D12Resource> _outlineRenderTarget;
	UINT _dsvHeapIndex = 0;
	UINT _srvHeapIndex = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE _rtvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE _dsvHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE _srvHandle;
};

