#pragma once

#include "E57.h"
#include "Triangle.h"

#include <chrono>
class ReconstructionAlgorithm
{
private:
	std::vector<Triangle> triangles;
protected:
	E57* e57;
	bool stopEarly;
	bool running;

	std::mutex triangleMutex;
public:
	virtual void Run(); // run the algorithm (edites triangles)
	virtual void SetUp(); // set up the algorithm, for example after loading new pointCloud
	virtual void Stop(); // stop algorithm early

	std::mutex& GetTriangleMutex();
	std::vector<Triangle>& GetTriangles();
	bool getRunning();

	ReconstructionAlgorithm(E57* e57);
	~ReconstructionAlgorithm();
};

