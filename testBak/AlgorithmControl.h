#pragma once

#include "MarchingCubes.h"
#include "Cubes.h"
#include "BallPivoting.h"

enum algorithms {
	CUBES, MARCHING_CUBES, BALL_PIVOTING
};

class AlgorithmControl
{
private:
	E57* e57;

	algorithms active;

	Cubes cubes;
	MarchingCubes marchinCubes;
	BallPivoting bp;

	ReconstructionAlgorithm* activeAlgorithm;
public:
	AlgorithmControl(E57* e57);
	AlgorithmControl(E57* e57, algorithms starting);

	void Run();
	void ChangeAlgorithm(algorithms a);
	void SetUp();

	std::vector<Triangle>& GetTriangles();

	~AlgorithmControl();
};

