#pragma once

#include "MarchingCubes.h"
#include "Cubes.h"
#include "BallPivoting.h"
#include "Poisson.h"

enum AlgorithmsEnum {
	CUBES, MARCHING_CUBES, BALL_PIVOTING, NONE
};

class AlgorithmControl
{
private:
	std::vector<Triangle> emptyVector;

	E57* e57;

	AlgorithmsEnum active;

	Cubes cubes;
	MarchingCubes marchinCubes;
	BallPivoting bp;

	ReconstructionAlgorithm* activeAlgorithm;
public:
	AlgorithmControl(E57* e);
	AlgorithmControl(E57* e, AlgorithmsEnum starting);

	void Run();
	void ChangeAlgorithm(AlgorithmsEnum a);
	void SetUp();
	void Stop();
	void ChangeParams(float* args);

	std::mutex* GetTriangleMutex();

	bool getRunning();

	AlgorithmsEnum GetActiveAlgorithm();
	std::vector<Triangle>& GetTriangles();

	~AlgorithmControl();
};

