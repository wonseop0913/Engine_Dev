#pragma once
#include <Jolt/Renderer/DebugRenderer.h>

#define DEFAULT_VERTEX_BUFFER_SIZE	10000000
#define DEFAULT_INDEX_BUFFER_SIZE	10000000

enum BULB_API LogLevel
{
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR
};

struct BULB_API DebugLog
{
	float time;
	LogLevel logLevel;
	string message;
};

struct BULB_API DebugLine
{
	DebugLine(float x1, float y1, float z1, float x2, float y2, float z2, float r, float g, float b, float a) {
		from = Bulb::Vector3(x1, y1, z1);
		to = Bulb::Vector3(x2, y2, z2);
		color = Bulb::Color(r, g, b, a);
	}

	DebugLine(Bulb::Vector3 _from, Bulb::Vector3 _to, Bulb::Color _color) {
		from = _from;
		to = _to;
		color = _color;
	}

	Bulb::Vector3 from;
	Bulb::Vector3 to;
	Bulb::Color color;
};

class TriangleBatch : public JPH::RefTargetVirtual {
public:
	vector<JPH::DebugRenderer::Triangle> triangles;

	virtual void AddRef() override { mRefCount++; }
	virtual void Release() override { if (--mRefCount == 0) delete this; }

private:
	std::atomic<uint32_t> mRefCount = 0;
};

class DebugRenderBodyFilter : public JPH::BodyDrawFilter {
public:
	bool ShouldDraw(const Body& inBody) const override;
};

class BULB_API DebugManager : public JPH::DebugRenderer
{
	JPH_OVERRIDE_NEW_DELETE
	using Super = JPH::DebugRenderer;

	friend class BulbApplication;
	friend class RenderManager;

private:
	DebugManager() = default;
	~DebugManager();

	void Init();
	void PreUpdate();
	void Update();
	void Render(ID3D12GraphicsCommandList* cmdList);

public:
	DebugManager(const DebugManager& rhs) = delete;
	DebugManager& operator=(const DebugManager& rhs) = delete;

	static DebugManager* GetInstance();
	static Bulb::ProcessResult Delete();

	void DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor) override;
	void DrawLine(Bulb::Vector3 from, Bulb::Vector3 to, Bulb::Color color);

	void DrawTriangle(RVec3Arg inV1, RVec3Arg inV2, RVec3Arg inV3, ColorArg inColor, ECastShadow inCastShadow = ECastShadow::Off) override;

	void DrawTriangle(Bulb::Vector3 v1, Bulb::Vector3 v2, Bulb::Vector3 v3, Bulb::Color color);

	Batch CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override;

	Batch CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const uint32* inIndices, int inIndexCount) override;
	
	void DrawGeometry(RMat44Arg inModelMatrix, const AABox& inWorldSpaceBounds, float inLODScaleSq, ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode = ECullMode::CullBackFace, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid) override;

	void DrawText3D(RVec3Arg inPosition, const string_view& inString, ColorArg inColor = Color::sWhite, float inHeight = 0.5f) override;

	bool IsPhysicsDebugRenderEnabled() { return _isPhysicsDebugRenderEnabled; }
	void SetPhysicsDebugRenderEnabled(bool value) { _isPhysicsDebugRenderEnabled = value; }

	void Log(const string& message);
	void WarnLog(const string& message);
	void ErrorLog(const string& message);
	void ClearLogs() { _debugLogs.clear(); }
	vector<DebugLog>& GetLogs() { return _debugLogs; }

	void ExportDebugLogs();

private:
	static DebugManager* s_instance;

	bool _isPhysicsDebugRenderEnabled = false;

	vector<DebugLog> _debugLogs;

	vector<VertexPC> _vertices;
	vector<UINT32> _indices;

	UINT _bufferVertexCount = 0;
	UINT _bufferIndexCount = 0;

	unique_ptr<UploadBuffer<VertexPC>> _vertexUploadBuffer;
	unique_ptr<UploadBuffer<UINT32>> _indexUploadBuffer;

	D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW _indexBufferView;

	Bulb::Color _colliderDebugColor = { 0.0f, 1.0f, 0.0f, 1.0f };

	const UINT32 _boxColliderIndices[24] = {
				0, 1,
				1, 2,
				2, 3,
				3, 0,
				0, 4,
				1, 5,
				2, 6,
				3, 7,
				4, 5,
				5, 6,
				6, 7,
				7, 4
	};

	DebugRenderBodyFilter* _debugRenderBodyFilter;
};

