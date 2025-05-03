#include "AlgorithmControl.h"

AlgorithmControl::AlgorithmControl(E57* e):e57(e), cubes(e), marchinCubes(e), bp(e), activeAlgorithm(nullptr)
{
	this->emptyVector = std::vector<Triangle>();
}

AlgorithmControl::AlgorithmControl(E57 * e, AlgorithmsEnum starting) :e57(e), cubes(e), marchinCubes(e), bp(e)
{
	this->emptyVector = std::vector<Triangle>();
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

void AlgorithmControl::ChangeAlgorithm(AlgorithmsEnum a)
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

void AlgorithmControl::Stop()
{
	if (this->activeAlgorithm)
		this->activeAlgorithm->Stop();
}

void AlgorithmControl::ChangeParams(float* args)
{		
	switch (this->active)
	{
	case CUBES:
	{
		// margin, voxelSize;
		float margin = args[0];
		float voxelSize = args[1];

		this->cubes.SetMargin(margin);
		this->cubes.SetVoxelSize(voxelSize);
		break;
	}
	case MARCHING_CUBES:
	{
		// isolevel, voxelSize;
		float isolevel = args[0];
		float voxelSize = args[1];
		float multi = args[2];
		int margin = (int)args[3];

		this->marchinCubes.SetIsoLevel(isolevel);
		this->marchinCubes.SetVoxelSize(voxelSize);
		this->marchinCubes.SetMargin(margin);
		this->marchinCubes.SetSigmaMultiplier(multi);
		break;
	}
	case BALL_PIVOTING:
	{
		// radius;
		float radius = args[0];

		this->bp.SetRadius(radius);
		break;
	}	
	}
}

std::mutex* AlgorithmControl::GetTriangleMutex()
{
	if (this->activeAlgorithm)
		return &this->activeAlgorithm->GetTriangleMutex();

	return nullptr;
}

bool AlgorithmControl::getRunning()
{
	if (this->activeAlgorithm)
		return this->activeAlgorithm->getRunning();

	return false;
}

AlgorithmsEnum AlgorithmControl::GetActiveAlgorithm()
{
	return this->active;
}

std::vector<Triangle>& AlgorithmControl::GetTriangles()
{
	if (this->activeAlgorithm)
		return this->activeAlgorithm->GetTriangles();

	return emptyVector;		
}

AlgorithmControl::~AlgorithmControl()
{
}
