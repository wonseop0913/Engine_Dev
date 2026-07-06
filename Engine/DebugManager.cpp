#include "pch.h"
#include "DebugManager.h"

DebugManager* DebugManager::s_instance = nullptr;

DebugManager::~DebugManager()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - DebugManager\n";
#endif

	_vertexUploadBuffer.reset();
	_indexUploadBuffer.reset();
}

DebugManager* DebugManager::GetInstance()
{
	if (s_instance == nullptr)
		s_instance = new DebugManager();
	return s_instance;
}

Bulb::ProcessResult DebugManager::Delete()
{
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
		return Bulb::ProcessResult::SUCCESS;
	}
	return Bulb::ProcessResult::FAILED_INSTANCE_NOT_FOUND;
}

void DebugManager::Init()
{
	_vertexBufferView.StrideInBytes = sizeof(VertexPC);
	_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	_bufferVertexCount = DEFAULT_VERTEX_BUFFER_SIZE;
	_vertexUploadBuffer = make_unique<UploadBuffer<VertexPC>>(DEFAULT_VERTEX_BUFFER_SIZE, false);

	_vertexBufferView.BufferLocation = _vertexUploadBuffer->GetResource()->GetGPUVirtualAddress();
	_vertexBufferView.SizeInBytes = sizeof(VertexPC) * DEFAULT_VERTEX_BUFFER_SIZE;

	_bufferIndexCount = DEFAULT_INDEX_BUFFER_SIZE;
	_indexUploadBuffer = make_unique<UploadBuffer<UINT32>>(DEFAULT_INDEX_BUFFER_SIZE, false);

	_indexBufferView.BufferLocation = _indexUploadBuffer->GetResource()->GetGPUVirtualAddress();
	_indexBufferView.SizeInBytes = sizeof(UINT32) * DEFAULT_INDEX_BUFFER_SIZE;

	Initialize();
}

void DebugManager::PreUpdate()
{
	_vertices.clear();
	_indices.clear();
}

void DebugManager::Update()
{
	if (_isPhysicsDebugRenderEnabled) {
		JPH::BodyManager::DrawSettings drawSettings;
		drawSettings.mDrawShape = true;
		drawSettings.mDrawShapeWireframe = true;
		PHYSICS->GetPhysicsSystem()->DrawBodies(drawSettings, this);
	}

	_vertexUploadBuffer->CopyData(_vertices.data(), _vertices.size());
	_indexUploadBuffer->CopyData(_indices.data(), _indices.size());
}

// Jolt 추가 이후 로직 수정이 필요함
void DebugManager::Render(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->IASetVertexBuffers(0, 1, &_vertexBufferView);
	cmdList->IASetIndexBuffer(&_indexBufferView);
	cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	cmdList->DrawIndexedInstanced(_indices.size(), 1, 0, 0, 0);
}

void DebugManager::DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor)
{
	_indices.push_back(_vertices.size());
	_indices.push_back(_vertices.size() + 1);

	_vertices.push_back(VertexPC(inFrom.GetX(), inFrom.GetY(), inFrom.GetZ(), inColor.r, inColor.g, inColor.b, inColor.a));
	_vertices.push_back(VertexPC(inTo.GetX(), inTo.GetY(), inTo.GetZ(), inColor.r, inColor.g, inColor.b, inColor.a));
}

void DebugManager::DrawLine(Bulb::Vector3 from, Bulb::Vector3 to, Bulb::Color color)
{
	_indices.push_back(_vertices.size());
	_indices.push_back(_vertices.size() + 1);

	_vertices.push_back(VertexPC(from, color));
	_vertices.push_back(VertexPC(to, color));
}

void DebugManager::DrawTriangle(RVec3Arg inV1, RVec3Arg inV2, RVec3Arg inV3, ColorArg inColor, ECastShadow inCastShadow)
{
	UINT32 start = _vertices.size();
	_indices.push_back(start);
	_indices.push_back(start + 1);
	_indices.push_back(start + 1);
	_indices.push_back(start + 2);
	_indices.push_back(start + 2);
	_indices.push_back(start);

	_vertices.push_back(VertexPC(inV1.GetX(), inV1.GetY(), inV1.GetZ(), inColor.r, inColor.g, inColor.b, inColor.a));
	_vertices.push_back(VertexPC(inV2.GetX(), inV2.GetY(), inV2.GetZ(), inColor.r, inColor.g, inColor.b, inColor.a));
	_vertices.push_back(VertexPC(inV3.GetX(), inV3.GetY(), inV3.GetZ(), inColor.r, inColor.g, inColor.b, inColor.a));
	//DrawLine(inV1, inV2, inColor);
	//DrawLine(inV2, inV3, inColor);
	//DrawLine(inV3, inV1, inColor);
}

void DebugManager::DrawTriangle(Bulb::Vector3 v1, Bulb::Vector3 v2, Bulb::Vector3 v3, Bulb::Color color)
{
	UINT32 start = _vertices.size();
	_indices.push_back(start);
	_indices.push_back(start + 1);
	_indices.push_back(start + 1);
	_indices.push_back(start + 2);
	_indices.push_back(start + 2);
	_indices.push_back(start);

	_vertices.push_back(VertexPC(v1, color));
	_vertices.push_back(VertexPC(v2, color));
	_vertices.push_back(VertexPC(v3, color));
}

JPH::DebugRenderer::Batch DebugManager::CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount)
{
	TriangleBatch* batch = new TriangleBatch;
	if (inTriangles == nullptr || inTriangleCount == 0)
		return batch;
	
	batch->triangles.assign(inTriangles, inTriangles + inTriangleCount);
	return batch;
}

JPH::DebugRenderer::Batch DebugManager::CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const uint32* inIndices, int inIndexCount)
{
	TriangleBatch* batch = new TriangleBatch;

	if (inVertices == nullptr || inVertexCount == 0 || inIndices == nullptr || inIndexCount == 0)
		return batch;

	size_t triangleCount = inIndexCount / 3;
	batch->triangles.resize(triangleCount);
	for (size_t t = 0; t < triangleCount; ++t) {
		batch->triangles.push_back(JPH::DebugRenderer::Triangle());
		batch->triangles[t].mV[0] = inVertices[inIndices[t * 3 + 0]];
		batch->triangles[t].mV[1] = inVertices[inIndices[t * 3 + 1]];
		batch->triangles[t].mV[2] = inVertices[inIndices[t * 3 + 2]];
	}

	return batch;
}

void DebugManager::DrawGeometry(RMat44Arg inModelMatrix, const AABox& inWorldSpaceBounds, float inLODScaleSq, ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode /*= ECullMode::CullBackFace*/, ECastShadow inCastShadow /*= ECastShadow::On*/, EDrawMode inDrawMode /*= EDrawMode::Solid*/)
{
	XMMATRIX world = XMLoadFloat4x4((XMFLOAT4X4*)&inModelMatrix);

	const TriangleBatch* triangleBatch = static_cast<const TriangleBatch *>(inGeometry->mLODs[0].mTriangleBatch.GetPtr());

	Bulb::Color color = { 0.0f, 1.0f, 0.0f, 1.0f };

	for (const JPH::DebugRenderer::Triangle& triangle : triangleBatch->triangles) {
		Bulb::Vector3 v0 = { triangle.mV[0].mPosition.x, triangle.mV[0].mPosition.y, triangle.mV[0].mPosition.z };
		Bulb::Vector3 v1 = { triangle.mV[1].mPosition.x, triangle.mV[1].mPosition.y, triangle.mV[1].mPosition.z };
		Bulb::Vector3 v2 = { triangle.mV[2].mPosition.x, triangle.mV[2].mPosition.y, triangle.mV[2].mPosition.z };
		v0 = v0 * world;
		v1 = v1 * world;
		v2 = v2 * world;

		DrawTriangle(v0, v1, v2, color);
	}
}

void DebugManager::DrawText3D(RVec3Arg inPosition, const string_view& inString, ColorArg inColor /*= Color::sWhite*/, float inHeight /*= 0.5f*/)
{

}

void DebugManager::Log(const string& message)
{
	_debugLogs.push_back({ TIME->TotalTime(), LOG_INFO, message });
}

void DebugManager::WarnLog(const string& message)
{
	_debugLogs.push_back({ TIME->TotalTime(), LOG_WARN, message });
}

void DebugManager::ErrorLog(const string& message)
{
	_debugLogs.push_back({ TIME->TotalTime(), LOG_ERROR, message });
}

void DebugManager::ExportDebugLogs()
{
	tinyxml2::XMLDocument doc;
	XMLElement* mainElem = doc.NewElement("DebugLogs");
	doc.InsertFirstChild(mainElem);

	for (DebugLog& log : _debugLogs) {
		XMLElement* logElem = mainElem->InsertNewChildElement("Log");
		logElem->SetText(log.message.c_str());
	}

	doc.SaveFile("DebugLog.xml");
}
