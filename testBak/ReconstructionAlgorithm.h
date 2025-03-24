#pragma once

#include "E57.h"
#include "Triangle.h"

class ReconstructionAlgorithm
{
private:
	std::vector<Triangle> triangles;
protected:
	E57* e57;
public:
	virtual void Run(); // run the algorithm (edites triangles)
	virtual void SetUp(); // set up the algorithm, for example after loading new pointCloud
	std::vector<Triangle>& getTriangles();

	ReconstructionAlgorithm(E57* e57);
	~ReconstructionAlgorithm();
};

