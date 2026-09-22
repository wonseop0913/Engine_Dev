#pragma once

#define DEFAULT_UI_COUNT 30

class BULB_API UIManager
{
	friend class BulbApplication;
	friend class RenderManager;

public:
	UIManager() = default;
	~UIManager();

	void Init();
	void Update();
	void Render(ID3D12GraphicsCommandList* cmdList);

public:
	UIManager(const UIManager& rhs) = delete;
	UIManager& operator=(const UIManager& rhs) = delete;

	static UIManager* GetInstance();
	static Bulb::ProcessResult Delete();

	void Initialize();
	void OnResolutionUpdate();

	template<typename T>
	shared_ptr<T> CreateUI(string name = "");

	UINT GetUIBufferSRVIndex() { return _uploadBufferSrvIndex; }

	void AddUI(const shared_ptr<UIElement>& ui);
	void DeleteUI(const shared_ptr<UIElement>& ui);

	void SetDepthSortFlag() { _sortFlag = true; }

	Bulb::Vector2 GetUIResolution() { return _uiResolution; }

private:
	void CreateBuffer();

private:
	static UIManager* s_instance;

	Bulb::Vector2 _uiResolution = { 1920.0f, 1080.0f };
	int _elementCount = 0;
	int _panelCount = 0;

	shared_ptr<Mesh> _quadMesh;
	shared_ptr<UIElement> _prevHoveredUI;
	shared_ptr<UIElement> _hoveredUI;

	vector<shared_ptr<UIElement>> _elements;
	vector<shared_ptr<UIPanel>> _panels;
	vector<UIInstanceConstants> _uiConstants;

	unique_ptr<UploadBuffer<UIInstanceConstants>> _uploadBuffer = nullptr;
	UINT _uploadBufferSrvIndex = -1;

	bool _sortFlag = false;
};

template<typename T>
shared_ptr<T> UIManager::CreateUI(string name)
{
	if (!is_base_of_v<UIElement, T>) {
		DEBUG->ErrorLog("[UIManager] - Incorrect UI class!");
		return nullptr;
	}

	shared_ptr<T> element = make_shared<T>();
	element->_transform = make_shared<UITransform>(element);
	element->Init();
	AddUI(element);

	return element;
}
