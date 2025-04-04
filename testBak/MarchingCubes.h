#pragma once
#include "MarchingCubeLookupTable.h"
#include "Triangle.h"
#include "ReconstructionAlgorithm.h"

#include <ctime>
#include <thread>
#include <mutex>


class MarchingCubes : public ReconstructionAlgorithm
{
private:
	std::mutex trianglesMutex;

	float minX, minY, minZ;

	int voxelsInDimX, voxelsInDimY, voxelsInDimZ;

	float isolevel;

	void GenerateCubeMesh(int x, int y, int z);

	void SetGridInRange(int startIdx, int endIdx);
	void GenerateMeshInRange(int startX, int endX, int startY, int endY, int startZ, int endZ);

	glm::vec3 InterpolateEdge(int x, int y, int z, int edge);
	void CreateTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c);

protected:
	float voxelSize;
	std::vector<std::vector<std::vector<float>>> grid;

	void GenerateMesh();
	void InitGrid();
	virtual void SetGrid();
	virtual float CalculateDensity(glm::vec3 point, glm::vec3 min, glm::vec3 index);
public:
	MarchingCubes(E57* e57);
	MarchingCubes(float isolevel, float voxelSize, E57* e57);

	void SetVoxelSize(float voxelSize);
	void SetIsoLevel(float isolevel);

	int GetVoxelsInDimX();
	int GetVoxelsInDimY();
	int GetVoxelsInDimZ();

	void Run() override;
	void SetUp() override;

	~MarchingCubes();
};

