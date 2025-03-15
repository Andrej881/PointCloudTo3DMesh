#include "MarginCubes.h"

MarginCubes::MarginCubes(float voxelSize, int margin,  std::vector<float>& points)
{
	this->margin = margin;
	this->voxelSize = voxelSize;

	this->voxelsInDim = ceil(1.0f / voxelSize) + 1;

	SetGrid(points);
}

void MarginCubes::SetGrid(std::vector<float>& points)
{
	this->cubes.clear();
	this->grid.clear();
	this->grid = std::vector<std::vector<std::vector<bool>>>(this->voxelsInDim, std::vector<std::vector<bool>>(this->voxelsInDim, std::vector<bool>(this->voxelsInDim, false)));
	//this->grid[0][0][0] = true;
	//this->grid[this->voxelsInDim-1][this->voxelsInDim-1][this->voxelsInDim-1] = true;
	// -0.5, 0.5
	for (int i = 0; i < points.size(); i = i + 3) {
		//printf("[%d] / [%d]\n", i, points.size());
		float x = points[i], y = points[i+1], z = points[i+2];

		float min = -0.5;
		int indexX = (x - min) / voxelSize, indexY = (y - min) / voxelSize, indexZ = (z - min) / voxelSize;

		if (indexX < 0 || indexY < 0 || indexZ < 0 || indexX >= this->voxelsInDim || indexY >= this->voxelsInDim || indexZ >= this->voxelsInDim)
		{
			//printf("X:%d Y:%d Z:%d voxels in dim:%d \n", indexX, indexY, indexZ, this->voxelsInDim);
			continue;
		}


		this->grid[indexX][indexY][indexZ] = true;
		for (int j1 = -margin; j1 <= margin; ++j1)
		{
			for (int j2 = -margin; j2 <= margin; ++j2)
			{
				for (int j3 = -margin; j3 <= margin; ++j3)
				{
					if ((j1 == 0 && j2 == 0 && j3 == 0) || (abs(j1) + abs(j2) + abs(j3) != margin))
						continue;
					int newIndexX = indexX + j1, newIndexY = indexY + j2, newIndexZ = indexZ + j3;
					if (newIndexX < 0 || newIndexY < 0 || newIndexZ < 0 || newIndexX >= this->voxelsInDim || newIndexY >= this->voxelsInDim || newIndexZ >= this->voxelsInDim)
					{
						//printf("X:%d Y:%d Z:%d voxels in dim:%d \n", newIndexX, newIndexY, newIndexZ, this->voxelsInDim);
						continue;
					}
					this->grid[newIndexX][newIndexY][newIndexZ] = true;
				}
			}
		}
	}
	GenerateMesh();
}

void MarginCubes::GenerateMesh()
{	
	float min = -0.5;
	for (int i1 = 0; i1 < this->voxelsInDim; i1++)
	{
		for (int i2 = 0; i2 < this->voxelsInDim; i2++)
		{
			for (int i3 = 0; i3 < this->voxelsInDim; i3++)
			{
				if (this->grid[i1][i2][i3]) 
				{
					CreateCube(i1, i2, i3);					
				}
			}
		}
	}
}

void MarginCubes::CreateCube(int x, int y, int z)
{
	Cube cube;

	cube.verteces[0] = { x * this->voxelSize, y * this->voxelSize, z * this->voxelSize };	
	cube.verteces[1] = { x * this->voxelSize + this->voxelSize, y * this->voxelSize, z * this->voxelSize };
	cube.verteces[2] = { x * this->voxelSize, y * this->voxelSize + this->voxelSize, z * this->voxelSize };
	cube.verteces[3] = { x * this->voxelSize, y * this->voxelSize, z * this->voxelSize + this->voxelSize };
	cube.verteces[4] = { x * this->voxelSize + this->voxelSize, y * this->voxelSize + this->voxelSize, z * this->voxelSize };
	cube.verteces[5] = { x * this->voxelSize, y * this->voxelSize + this->voxelSize, z * this->voxelSize + this->voxelSize };
	cube.verteces[6] = { x * this->voxelSize + this->voxelSize, y * this->voxelSize, z * this->voxelSize + this->voxelSize };
	cube.verteces[7] = { x * this->voxelSize + this->voxelSize, y * this->voxelSize + this->voxelSize, z * this->voxelSize + this->voxelSize };
	
	//front (z-)
	if (z <= 0 || (z > 0 && !this->grid[x][y][z - 1]))
	{
		cube.sides[0].active = true;
		cube.sides[0].triangles[0] = {cube.verteces[0] , cube.verteces[1] , cube.verteces[2]};
		cube.sides[0].triangles[1] = { cube.verteces[1] , cube.verteces[4] , cube.verteces[2] };

		cube.sides[0].triangles[0].computeNormal();
		cube.sides[0].triangles[1].computeNormal();

		this->numOfTriangels += 2;
	}

	//leftSide (x-)
	if (x <= 0 || (x > 0 && !this->grid[x - 1][y][z]))
	{
		cube.sides[1].active = true;
		cube.sides[1].triangles[0] = { cube.verteces[0] , cube.verteces[2] , cube.verteces[3] };
		cube.sides[1].triangles[1] = { cube.verteces[2] , cube.verteces[5] , cube.verteces[3] };

		cube.sides[1].triangles[0].computeNormal();
		cube.sides[1].triangles[1].computeNormal();

		this->numOfTriangels += 2;
	}

	//rightSide (x+)
	if (x >= this->voxelsInDim - 1 || (x < this->voxelsInDim - 1 && !this->grid[x + 1][y][z]))
	{
		cube.sides[2].active = true;
		cube.sides[2].triangles[0] = { cube.verteces[1] , cube.verteces[6] , cube.verteces[4] };
		cube.sides[2].triangles[1] = { cube.verteces[4] , cube.verteces[6] , cube.verteces[7] };

		cube.sides[2].triangles[0].computeNormal();
		cube.sides[2].triangles[1].computeNormal();

		this->numOfTriangels += 2;
	}

	//back (z+)
	if (z >= this->voxelsInDim - 1 || (z < this->voxelsInDim - 1 && !this->grid[x][y][z + 1]))
	{
		cube.sides[3].active = true;
		cube.sides[3].triangles[0] = { cube.verteces[3] , cube.verteces[7] , cube.verteces[6] };
		cube.sides[3].triangles[1] = { cube.verteces[3] , cube.verteces[5] , cube.verteces[7] };

		cube.sides[3].triangles[0].computeNormal();
		cube.sides[3].triangles[1].computeNormal();

		this->numOfTriangels += 2;
	}

	//up (y+)
	if (y >= this->voxelsInDim - 1 || (y < this->voxelsInDim - 1 && !this->grid[x][y + 1][z]))
	{
		cube.sides[4].active = true;
		cube.sides[4].triangles[0] = { cube.verteces[2] , cube.verteces[4] , cube.verteces[5] };
		cube.sides[4].triangles[1] = { cube.verteces[4] , cube.verteces[7] , cube.verteces[5] };

		cube.sides[4].triangles[0].computeNormal();
		cube.sides[4].triangles[1].computeNormal();

		this->numOfTriangels += 2;
	}

	//down (y-)
	if (y <= 0 || (y > 0 && !this->grid[x][y - 1][z]))
	{
		cube.sides[5].active = true;
		cube.sides[5].triangles[0] = { cube.verteces[0] , cube.verteces[3] , cube.verteces[1] };
		cube.sides[5].triangles[1] = { cube.verteces[1] , cube.verteces[3] , cube.verteces[6] };

		cube.sides[5].triangles[0].computeNormal();
		cube.sides[5].triangles[1].computeNormal();

		this->numOfTriangels += 2;
	}

	/*
	//tmp
	//front
	cube.triangles[0] = { cube.verteces[0] , cube.verteces[1] , cube.verteces[2] }; //012
	cube.triangles[1] = { cube.verteces[1] , cube.verteces[2] , cube.verteces[4] }; //124

	//down
	cube.triangles[2] = { cube.verteces[0] , cube.verteces[1] , cube.verteces[3] }; //013
	cube.triangles[3] = { cube.verteces[1] , cube.verteces[3] , cube.verteces[6] }; //136

	//leftSide
	cube.triangles[4] = { cube.verteces[0] , cube.verteces[2] , cube.verteces[3] }; //023
	cube.triangles[6] = { cube.verteces[2] , cube.verteces[3] , cube.verteces[5] }; //235

	//rightSide
	cube.triangles[6] = { cube.verteces[1] , cube.verteces[4] , cube.verteces[6] }; //146
	cube.triangles[7] = { cube.verteces[4] , cube.verteces[6] , cube.verteces[7] }; //467

	//back
	cube.triangles[8] = { cube.verteces[3] , cube.verteces[6] , cube.verteces[7] }; //367
	cube.triangles[9] = { cube.verteces[3] , cube.verteces[5] , cube.verteces[7] }; //357

	//up
	cube.triangles[10] = { cube.verteces[2] , cube.verteces[4] , cube.verteces[5] }; //245
	cube.triangles[11] = { cube.verteces[4] , cube.verteces[5] , cube.verteces[7] }; //457
	*/
	this->cubes.push_back(cube);
}

std::vector<Cube>& MarginCubes::getCubes()
{
	return this->cubes;
}

MarginCubes::~MarginCubes()
{
}
