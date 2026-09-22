#pragma once

struct Vertex;

class GeometryGenerator
{
public:
	static shared_ptr<Geometry> CreateBox(float width, float height, float depth, UINT32 numSubdivisions);
	static shared_ptr<Geometry> CreateGeosphere(float radius, UINT32 numSubdivisions);
	static shared_ptr<Geometry> CreateQuad();
	static shared_ptr<Geometry> CreateQuadTwoSided();
	static shared_ptr<Geometry> CreateTerrain(UINT oneSideSampleCount);

	static void Subdivide(shared_ptr<Geometry> mesh);
	static Vertex MidPoint(const Vertex& v0, const Vertex& v1);
};
