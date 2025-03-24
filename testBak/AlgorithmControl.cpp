#include "AlgorithmControl.h"

AlgorithmControl::AlgorithmControl(E57* e57):e57(e57), cubes(e57), marchinCubes(e57), bp(e57), activeAlgorithm(nullptr)
{}

AlgorithmControl::AlgorithmControl(E57 * e57, algorithms starting) :e57(e57), cubes(e57), marchinCubes(e57), bp(e57)
{
	this->active = starting;
	switch (starting)
	{
	case CUBES:
		this->activeAlgorithm = &cubes;
		break;
	case MARCHING_CUBES:
		this->activeAlgorithm = &marchinCubes;
		break;
	case BALL_PIVOTING:
		this->activeAlgorithm = &bp;
		break;
	}
	SetUp();
	Run();
}

void AlgorithmControl::Run()
{
	if (this->activeAlgorithm)
		this->activeAlgorithm->Run();
}

void AlgorithmControl::ChangeAlgorithm(algorithms a)
{
	if (this->active == a)	
		return;
	
	this->active = a;
	switch (a)
	{
	case CUBES:
		this->activeAlgorithm = &cubes;
		break;
	case MARCHING_CUBES:
		this->activeAlgorithm = &marchinCubes;
		break;
	case BALL_PIVOTING:
		this->activeAlgorithm = &bp;
		break;
	}
	SetUp();
}

void AlgorithmControl::SetUp()
{
	if (this->activeAlgorithm)
		this->activeAlgorithm->SetUp();
}

std::vector<Triangle>& AlgorithmControl::GetTriangles()
{
	if (this->activeAlgorithm)
		return this->activeAlgorithm->getTriangles();
	else 
		return cubes.getTriangles();
		
}

AlgorithmControl::~AlgorithmControl()
{
}
