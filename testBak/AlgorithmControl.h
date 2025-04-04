#pragma once

#include "MarchingCubes.h"
#include "Cubes.h"
#include "BallPivoting.h"
#include "Poisson.h"

enum algorithms {
	CUBES, MARCHING_CUBES, BALL_PIVOTING, POISSON
};

class AlgorithmControl
{
private:
	E57* e57;

	algorithms active;

	Cubes cubes;
	MarchingCubes marchinCubes;
	Poisson poisson;
	BallPivoting bp;

	ReconstructionAlgorithm* activeAlgorithm;
public:
	AlgorithmControl(E57* e);
	AlgorithmControl(E57* e, algorithms starting);

	void Run();
	void ChangeAlgorithm(algorithms a);
	void SetUp();

	std::vector<Triangle>& GetTriangles();

	~AlgorithmControl();
};

