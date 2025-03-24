#pragma once

#include "E57.h"
#include <vector>

#include "ReconstructionAlgorithm.h"

struct Side {
	bool active = false;
	Triangle triangles[2];
};

struct Cube
{
	E57Point verteces[8];

	Side sides[6]; // front, leftSide, rightSide, back, up, down
	
	//tmp
	Triangle triangles[12];
};

class Cubes : public ReconstructionAlgorithm
{
private:
	float voxelSize;
	int margin;
	float minX, minY, minZ;

	int voxelsInDimX, voxelsInDimY, voxelsInDimZ;
	std::vector<std::vector<std::vector<bool>>> grid;

	void GenerateMesh();
	void CreateCube(int x, int y, int z);
	void InitGrid();
	void SetGrid();
public:
	Cubes(E57 * e57);
	Cubes(float voxelSize, int margin, E57* e57);

	void SetVoxelSize(float voxelSize);
	void SetMargin(int margin);

	void Run() override;
	void SetUp() override;

	~Cubes();
};

