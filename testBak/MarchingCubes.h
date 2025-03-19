#pragma once
#include "Cubes.h"
#include "MarchingCubeLookupTable.h"

#include <ctime>
#include <thread>
#include <mutex>


class MarchingCubes
{
private:
	std::mutex trianglesMutex;

	float voxelSize;
	int margin;

	int voxelsInDimX, voxelsInDimY, voxelsInDimZ;
	std::vector<std::vector<std::vector<float>>> grid; 
	std::vector<Triangle> triangles;

	float isolevel = 0.25f;

	float CalculateDensity(glm::vec3 point, glm::vec3 min, glm::vec3 index);
	void GenerateMesh(E57* e57);
	void GenerateCubeMesh(int x, int y, int z, E57* e57);

	void SetGridInRange(std::vector<float>& points, E57* e57, int startIdx, int endIdx);
	void GenerateMeshInRange(int startX, int endX, int startY, int endY, int startZ, int endZ, E57* e57);
public:
	void InitGrid(E57* e57);
	MarchingCubes(float voxelSize, int margin, E57& e57);
	MarchingCubes(float voxelSize, int margin, std::vector<float>& points);
	void SetGrid(std::vector<float>& points, E57* e57);
	glm::vec3 InterpolateEdge(int x, int y, int z, int edge, E57* e57);
	std::vector<Triangle>& getTriangles();
	void CreateTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c);
	~MarchingCubes();
};

