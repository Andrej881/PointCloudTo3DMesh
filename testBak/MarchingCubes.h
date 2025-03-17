#pragma once
#include "Cubes.h"
#include "MarchingCubeLookupTable.h"
class MarchingCubes
{
private:
	float voxelSize;
	int margin;

	int voxelsInDimX, voxelsInDimY, voxelsInDimZ;
	std::vector<std::vector<std::vector<bool>>> grid; //density values instead of just bool
	std::vector<Triangle> triangles;

	void GenerateMesh();
	void GenerateCubeMesh(int x, int y, int z);
public:
	void InitGrid(std::vector<float>& points, E57* e57);
	MarchingCubes(float voxelSize, int margin, E57& e57);
	MarchingCubes(float voxelSize, int margin, std::vector<float>& points);
	void SetGrid(std::vector<float>& points, E57* e57);
	glm::vec3 InterpolateEdge(int x, int y, int z, int edge);
	std::vector<Triangle>& getTriangles();
	void CreateTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c);
	~MarchingCubes();
};

