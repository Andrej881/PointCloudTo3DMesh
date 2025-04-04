#pragma once

#include "ReconstructionAlgorithm.h"
#include "Octree.h"
#include "MarchingCubes.h"
#include <unordered_map>

class Poisson : public MarchingCubes
{
private:
	int n;
	int numOfIter;
	int depth;
	Octree tree;

	void FillTree();
	void SampleOctreeToGrid(OctreeNode* node, int depth);
	void SetGrid() override;
	float GetDensity(OctreeNode* node, glm::vec3 position);

	void IterateGaussSeidel(OctreeNode * node);
	std::vector<std::vector<std::vector<float>>> scalarField;
protected:
	float CalculateDensity(glm::vec3 point, glm::vec3 min, glm::vec3 index) override;
public:
	Poisson(E57* e57);
	Poisson(E57* e57, int n, int numOfIter, int depth, E57Point center, float size, float voxelSize, int margin, float isolevel);

	std::vector<std::vector<std::vector<float>>>& GetField();

	void SetUp() override;
	void Run() override;
};

