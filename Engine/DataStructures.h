#pragma once
#include "pch.h"

class Geometry;
class GameObject;
class Transform;

struct BULB_API AppDesc
{
	wstring mainWndCaption = L"Bulb Application";
	bool _4xMsaaState = false;
	int _4xMsaaQuality = 0;
	int clientWidth = 1280;
	int clientHeight = 720;
};

struct BULB_API AppStatus
{
	bool		appPaused = false;
	bool		minimized = false;
	bool		maximized = false;
	bool		resizing = false;
	bool		fullscreenState = false;
};

struct NodeTempData
{
	string name;
	UINT id;
	shared_ptr<NodeTempData> parent;
	XMFLOAT4X4 transform;
	XMFLOAT4X4 parentTransform;
};

struct BoneData
{
	string name;
	UINT id;
	UINT parentId = 0;
	XMFLOAT4X4 offsetTransform;
	XMFLOAT4X4 localBindTransform;
	shared_ptr<NodeTempData> node;
	shared_ptr<GameObject> instancedObj;
	shared_ptr<Transform> instancedTransform;
};

struct BoneWeight
{
	UINT vertexIndex;
	float weight;
};

struct Vertex
{
	Vertex() {}
	Vertex(
		const Bulb::Vector3& p,
		const Bulb::Vector3& n,
		const Bulb::Vector3& t,
		const Bulb::Vector2& uv) :
		Position(p),
		Normal(n),
		Tangent(t),
		TexC(uv) {}
	Vertex(
		float px, float py, float pz,
		float nx, float ny, float nz,
		float tx, float ty, float tz,
		float u, float v) :
		Position(px, py, pz),
		Normal(nx, ny, nz),
		Tangent(tx, ty, tz),
		TexC(u, v) {}

	void AddWeight(UINT index, float weight)
	{
		for (int i = 3; i > 0; i--)
			boneIndices[i] = boneIndices[i - 1];
		boneIndices[0] = index;

		boneWeights.w = boneWeights.z;
		boneWeights.z = boneWeights.y;
		boneWeights.y = boneWeights.x;

		boneWeights.x = weight;
	}

	Bulb::Vector3 Position;
	Bulb::Vector3 Normal;
	Bulb::Vector3 Tangent;
	Bulb::Vector2 TexC;
	Bulb::Vector4 boneWeights;
	INT boneIndices[4] = { -1, -1, -1, -1 };
};

struct VertexP
{
	VertexP() {};
	VertexP(const Bulb::Vector3& p) : Position(p) {}
	VertexP(float px, float py, float pz) : Position(px, py, pz) {}

	Bulb::Vector3 Position;
};

struct VertexPC
{
	VertexPC() {};
	VertexPC(
		const Bulb::Vector3& p, 
		const Bulb::Color& c) :
		Position(p), 
		Color(c) {}
	VertexPC(
		float px, float py, float pz, 
		float cr, float cg, float cb, float ca) : 
		Position(px, py, pz), 
		Color(cr, cg, cb, ca) {}

	Bulb::Vector3 Position;
	Bulb::Color Color;
};

struct VertexPT
{
	VertexPT() {};
	VertexPT(
		const Bulb::Vector3& p, 
		const Bulb::Vector2& t) : 
		Position(p), 
		TexC(t) {}
	VertexPT(
		float px, float py, float pz, 
		float u, float v) : 
		Position(px, py, pz), 
		TexC(u, v) {}
	Bulb::Vector3 Position;
	Bulb::Vector2 TexC;
};

struct ComponentSnapshot {
	int id;
	string componentType;
	vector<float> datas;
	vector<string> strDatas;
};

struct GameObjectSnapshot {
	int id;
	string name;
	string pso;
	vector<int> compSnapshotIndices;
	bool isActive;
};
