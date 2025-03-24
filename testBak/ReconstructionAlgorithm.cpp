#include "ReconstructionAlgorithm.h"

void ReconstructionAlgorithm::Run()
{
}

void ReconstructionAlgorithm::SetUp()
{
}

std::vector<Triangle>& ReconstructionAlgorithm::getTriangles()
{
    return this->triangles;
}

ReconstructionAlgorithm::ReconstructionAlgorithm(E57 * e57) : e57(e57)
{
	this->triangles = std::vector<Triangle>();
}

ReconstructionAlgorithm::~ReconstructionAlgorithm()
{
}
