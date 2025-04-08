#pragma once

#include "MarchingCubes.h"
#include "Cubes.h"
#include "BallPivoting.h"
#include "Poisson.h"

enum algorithmsEnum {
	CUBES, MARCHING_CUBES, BALL_PIVOTING, POISSON, NONE
};

class AlgorithmControl
{
private:
	std::vector<Triangle> emptyVector;

	E57* e57;

	algorithmsEnum active;

	Cubes cubes;
	MarchingCubes marchinCubes;
	Poisson poisson;
	BallPivoting bp;

	ReconstructionAlgorithm* activeAlgorithm;
public:
	AlgorithmControl(E57* e);
	AlgorithmControl(E57* e, algorithmsEnum starting);

	void Run();
	void ChangeAlgorithm(algorithmsEnum a);
	void SetUp();
	void Stop();
	void ChangeParams(float* args);

	std::mutex* GetTriangleMutex();

	bool getRunning();

	algorithmsEnum GetActiveAlgorithm();
	std::vector<Triangle>& GetTriangles();

	~AlgorithmControl();
};

