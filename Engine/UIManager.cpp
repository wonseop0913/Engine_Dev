#include "pch.h"
#include "UIManager.h"

UIManager* UIManager::s_instance = nullptr;

UIManager::~UIManager()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - UIManager\n";
#endif

	_quadMesh.reset();
	_prevHoveredUI.reset();
	_hoveredUI.reset();

	for (auto& e : _elements) {
		e->OnDestroy();
	}

	_elements.clear();
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
	_elements.clear();
	_panels.clear();
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
	if (_sortFlag) {
		sort(_elements.begin(), _elements.end(), [](shared_ptr<UIElement> a, shared_ptr<UIElement> b) -> bool { return a->GetTransform()->GetDepth() > b->GetTransform()->GetDepth(); });
		sort(_panels.begin(), _panels.end(), [](shared_ptr<UIPanel> a, shared_ptr<UIPanel> b) -> bool { return a->GetTransform()->GetDepth() > b->GetTransform()->GetDepth(); });
		_sortFlag = false;
	}

	D3D12_VIEWPORT viewport = GRAPHIC->GetViewport();
	XMMATRIX viewProj = XMLoadFloat4x4(&Camera::GetViewProjMatrix());

	POINT mousePos = INPUTM->GetMousePosition();
	mousePos.x = mousePos.x - viewport.Width / 2.0f;
	mousePos.y = -mousePos.y + viewport.Height / 2.0f;

	_uiConstants.clear();

	_prevHoveredUI = _hoveredUI;
	_hoveredUI = nullptr;
	for (auto& ui : _elements) {
		ui->Update();

		if (ui->_passthroughMouse) continue;

		if (ui->GetTransform()->CheckInRect(mousePos.x, mousePos.y)) {
			if (_hoveredUI == nullptr)
				_hoveredUI = ui;
			else {
				if (ui->GetTransform()->GetDepth() <= _hoveredUI->GetTransform()->GetDepth()) {
					_hoveredUI = ui;
				}
			}
		}
	}

	if (_prevHoveredUI != _hoveredUI) {
		if (_prevHoveredUI != nullptr) _prevHoveredUI->OnMouseExit();
		if (_hoveredUI != nullptr) _hoveredUI->OnMouseEnter();
	}
	if (_hoveredUI != nullptr) {
		if (INPUTM->IsMouseLeftButtonDown()) _hoveredUI->OnMouseDown();
	}

	for (auto& ui : _panels) {
		UIInstanceConstants constants;
		shared_ptr<UITransform> transform = ui->GetTransform();
		Bulb::Vector3 position = transform->GetPosition();
		Bulb::Vector2 size = transform->GetSize();
		Bulb::Vector2 pivot = transform->GetPivot();

		constants.CenterPos = { position.x, position.y };

		constants.Color = ui->IsRenderActive() ? ui->_color : XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		constants.Depth = transform->GetDepth();
		constants.TextureIndex = ui->_textureSrvHeapIndex;
		constants.Size = size;

		_uiConstants.push_back(constants);
	}

	_uploadBuffer->CopyData(_uiConstants.data(), _panels.size());
}

void UIManager::Render(ID3D12GraphicsCommandList* cmdList)
{
	if (_panelCount <= 0) return;

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
	++_elementCount;
	_elements.push_back(ui);
	if (ui->_type & (UINT)UIType::Panel) {
		++_panelCount;
		_panels.push_back(static_pointer_cast<UIPanel>(ui));
	}

	_sortFlag = true;
}

void UIManager::DeleteUI(const shared_ptr<UIElement>& ui)
{
	for (int i = 0; i < _elementCount; ++i) {
		if (_elements[i] == ui) {
			--_elementCount;
			_elements.erase(_elements.begin() + i);
		}
	}

	if (ui->_type & (UINT)UIType::Panel) {
		for (int i = 0; i < _panels.size(); ++i) {
			if (_panels[i] == ui) {
				_panels.erase(_panels.begin() + i);
			}
		}
	}
}
