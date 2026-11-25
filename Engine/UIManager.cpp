#include "pch.h"
#include "UIManager.h"
#include <unordered_set>

UIManager* UIManager::s_instance = nullptr;

UIManager::~UIManager()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - UIManager\n";
#endif

	_quadMesh.reset();
	_prevHoveredUI.reset();
	_hoveredUI.reset();
	_pressedUI.reset();

	for (auto& e : _elements) {
		e->OnDestroy();
	}

	_elements.clear();
	_orderedElements.clear();
	_panels.clear();
}

UIManager* UIManager::GetInstance()
{
	if (s_instance == nullptr)
		s_instance = new UIManager();
	return s_instance;
}

Bulb::ProcessResult UIManager::Delete()
{
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
		return Bulb::ProcessResult::SUCCESS;
	}
	return Bulb::ProcessResult::FAILED_INSTANCE_NOT_FOUND;
}

void UIManager::Initialize()
{
	_pressedUI.reset();
	_prevHoveredUI.reset();
	_hoveredUI.reset();
	_elements.clear();
	_orderedElements.clear();
	_panels.clear();
}

void UIManager::OnResolutionUpdate()
{
	for (auto& ui : _elements) {
		ui->GetTransform()->OnResolutionUpdate();
	}
}

void UIManager::Init()
{
	_quadMesh = RESOURCE->Get<Mesh>(DEFAULT_MESH_QUAD);
	_quadMesh->CreateBuffer();

	CreateBuffer();
}

// dirty flag를 넣던가 해서 최적화 작업 필요
void UIManager::Update()
{
	_orderedElements.clear();
	_panels.clear();
	unordered_set<UIElement*> registeredElements;
	for (const auto& ui : _elements)
		registeredElements.insert(ui.get());

	auto appendHierarchy = [&](auto&& self, const shared_ptr<UIElement>& ui) -> void {
		if (ui == nullptr || registeredElements.erase(ui.get()) == 0)
			return;

		_orderedElements.push_back(ui);
		if (ui->_type & (UINT)UIType::Panel)
			_panels.push_back(static_pointer_cast<UIPanel>(ui));

		for (const auto& childTransform : ui->GetTransform()->GetChilds())
			self(self, childTransform->GetUIElement());
	};

	for (const auto& ui : _elements) {
		if (ui->GetTransform()->GetParent() == nullptr)
			appendHierarchy(appendHierarchy, ui);
	}
	// Include any elements whose transform was not reachable from a registered root.
	//for (const auto& ui : _elements)
	//	appendHierarchy(appendHierarchy, ui);

	D3D12_VIEWPORT viewport = GRAPHIC->GetViewport();
	XMMATRIX viewProj = XMLoadFloat4x4(&Camera::GetViewProjMatrix());

	POINT mousePos = INPUTM->GetMousePosition();
	mousePos.x = mousePos.x - viewport.Width / 2.0f;
	mousePos.y = -mousePos.y + viewport.Height / 2.0f;

	_uiConstants.clear();

	_prevHoveredUI = _hoveredUI;
	_hoveredUI = nullptr;
	for (auto& ui : _orderedElements)
		ui->Update();

	for (auto ui = _orderedElements.rbegin(); ui != _orderedElements.rend(); ++ui) {
		if ((*ui)->_passthroughMouse || !(*ui)->IsRenderActive())
			continue;
		if ((*ui)->GetTransform()->CheckInRect(mousePos.x, mousePos.y)) {
			_hoveredUI = *ui;
			break;
		}
	}

	if (_prevHoveredUI != _hoveredUI) {
		if (_prevHoveredUI != nullptr) _prevHoveredUI->OnMouseExit();
		if (_hoveredUI != nullptr) _hoveredUI->OnMouseEnter();
	}
	if (_hoveredUI != nullptr) {
		if (INPUTM->IsMouseLeftButtonDown()) {
			_pressedUI = _hoveredUI;
			_pressedUI->OnMouseDown();
		}
	}

	if (_pressedUI != nullptr && INPUTM->IsMouseLeftButtonUp()) {
		if (_pressedUI == _hoveredUI)
			_pressedUI->OnMouseUp();
		_pressedUI.reset();
	}

	for (auto& ui : _panels) {
		UIInstanceConstants constants;
		shared_ptr<UITransform> transform = ui->GetTransform();
		Bulb::Vector3 position = transform->GetPosition();
		Bulb::Vector2 size = transform->GetSize();
		Bulb::Vector2 pivot = transform->GetPivot();

		constants.CenterPos = { position.x, position.y };

		constants.Color = ui->IsRenderActive() ? ui->_color : XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		constants.TextureIndex = ui->_textureSrvHeapIndex;
		constants.Size = size;

		_uiConstants.push_back(constants);
	}

	_uploadBuffer->CopyData(_uiConstants.data(), _panels.size());
}

void UIManager::Render(ID3D12GraphicsCommandList* cmdList)
{
	if (_panels.empty()) return;

	cmdList->IASetVertexBuffers(0, 1, &_quadMesh->vertexBufferView);
	cmdList->IASetIndexBuffer(&_quadMesh->indexBufferView);
	cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	cmdList->DrawIndexedInstanced(_quadMesh->GetIndexCount(), _panels.size(), 0, 0, 0);
}

void UIManager::CreateBuffer()
{
	// Create Buffer
	if (_uploadBuffer != nullptr)
		_uploadBuffer.release();
	_uploadBuffer = make_unique<UploadBuffer<UIInstanceConstants>>(DEFAULT_UI_COUNT, false);

	// Create SRV
	_uploadBufferSrvIndex = GRAPHIC->GetAndIncreaseSRVHeapIndex();
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(GRAPHIC->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart());
	hDescriptor.Offset(_uploadBufferSrvIndex, GRAPHIC->GetCBVSRVDescriptorSize());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = DEFAULT_UI_COUNT;
	srvDesc.Buffer.StructureByteStride = sizeof(UIInstanceConstants);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	GRAPHIC->GetDevice()->CreateShaderResourceView(_uploadBuffer->GetResource(), &srvDesc, hDescriptor);
}

void UIManager::AddUI(const shared_ptr<UIElement>& ui)
{
	_elements.push_back(ui);
}

void UIManager::DeleteUI(const shared_ptr<UIElement>& ui)
{
	if (_pressedUI == ui) {
		_pressedUI.reset();
	}
	if (_hoveredUI == ui) _hoveredUI.reset();
	if (_prevHoveredUI == ui) _prevHoveredUI.reset();

	_elements.erase(remove(_elements.begin(), _elements.end(), ui), _elements.end());
}
