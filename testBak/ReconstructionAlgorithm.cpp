#include "ReconstructionAlgorithm.h"

void ReconstructionAlgorithm::Run()
{	
}

void ReconstructionAlgorithm::SetUp()
{
}

void ReconstructionAlgorithm::Stop()
{
	this->stopEarly = true;
}

std::vector<Triangle>& ReconstructionAlgorithm::GetTriangles()
{
    return this->triangles;
}

bool ReconstructionAlgorithm::getRunning()
{
	return this->running;
}

ReconstructionAlgorithm::ReconstructionAlgorithm(E57 * e57) : e57(e57)
{
	this->triangles = std::vector<Triangle>();
	this->stopEarly = this->running = false;
}

ReconstructionAlgorithm::~ReconstructionAlgorithm()
{
}
