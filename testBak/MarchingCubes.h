#pragma once
#include "Cubes.h"
#include "MarchingCubeLookupTable.h"
class MarchingCubes
{
private:
	float voxelSize;
	int margin;

	int voxelsInDimX, voxelsInDimY, voxelsInDimZ;
	std::vector<std::vector<std::vector<float>>> grid; //density values instead of just bool
	std::vector<Triangle> triangles;

	float isolevel = 0.2f;

	float CalculateDensity(float distance, float sigma);
	void GenerateMesh(E57* e57);
	void GenerateCubeMesh(int x, int y, int z, E57* e57);
public:
	void InitGrid(std::vector<float>& points, E57* e57);
	MarchingCubes(float voxelSize, int margin, E57& e57);
	MarchingCubes(float voxelSize, int margin, std::vector<float>& points);
	void SetGrid(std::vector<float>& points, E57* e57);
	glm::vec3 InterpolateEdge(int x, int y, int z, int edge, E57* e57);
	std::vector<Triangle>& getTriangles();
	void CreateTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c);
	~MarchingCubes();
};

