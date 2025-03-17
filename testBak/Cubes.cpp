#include "Cubes.h"

void Cubes::InitGrid(std::vector<float>& points, E57* e57)
{
	this->cubes.clear();
	this->grid.clear();
	if (e57)
	{
		NormilizedPointsInfo& info = e57->getInfo();
		this->voxelsInDimX = (int)((info.maxX - info.minX) / voxelSize) + 1;
		this->voxelsInDimY = (int)((info.maxY - info.minY) / voxelSize) + 1;
		this->voxelsInDimZ = (int)((info.maxZ - info.minZ) / voxelSize) + 1;
	}
	else
	{
		this->voxelsInDimX = this->voxelsInDimY = this->voxelsInDimZ = (int)(1.0f / voxelSize) + 1;
	}
	this->grid = std::vector<std::vector<std::vector<bool>>>(this->voxelsInDimX, std::vector<std::vector<bool>>(this->voxelsInDimY, std::vector<bool>(this->voxelsInDimZ, false)));
}

Cubes::Cubes(float voxelSize, int margin, E57& e57)
{
	this->margin = margin;
	this->voxelSize = voxelSize;

	InitGrid(e57.getPoints(), &e57);
	SetGrid(e57.getPoints(), &e57);
}

Cubes::Cubes(float voxelSize, int margin,  std::vector<float>& points)
{
	this->margin = margin;
	this->voxelSize = voxelSize;

	InitGrid(points, nullptr);
	SetGrid(points, nullptr);
}

void Cubes::SetGrid(std::vector<float>& points, E57* e57)
{
	//this->grid[0][0][0] = true;
	//this->grid[this->voxelsInDim-1][this->voxelsInDim-1][this->voxelsInDim-1] = true;
	// -0.5, 0.5

	for (int i = 0; i < points.size(); i = i + 3) {
		//printf("[%d] / [%d]\n", i, points.size());
		float x = points[i], y = points[i+1], z = points[i+2];

		float minX, minY, minZ;
		if (e57)
		{
			minX = e57->getInfo().minX;
			minY = e57->getInfo().minY;
			minZ = e57->getInfo().minZ;
		}
		else
		{
			minX = minY = minZ = -0.5;
		}
		int indexX = (x - minX) / voxelSize, indexY = (y - minY) / voxelSize, indexZ = (z - minZ) / voxelSize;

		if (indexX < 0 || indexY < 0 || indexZ < 0 || indexX >= this->voxelsInDimX || indexY >= this->voxelsInDimY || indexZ >= this->voxelsInDimZ)
		{
			//printf("X:%d Y:%d Z:%d voxels in dim:%d \n", indexX, indexY, indexZ, this->voxelsInDim);
			continue;
		}

		if (margin == 0)
		{
			this->grid[indexX][indexY][indexZ] = true;
		}

		for (int j1 = -margin; j1 <= margin; ++j1)
		{
			for (int j2 = -margin; j2 <= margin; ++j2)
			{
				for (int j3 = -margin; j3 <= margin; ++j3)
				{
					if ((j1 == 0 && j2 == 0 && j3 == 0) || (abs(j1) + abs(j2) + abs(j3) != margin))
						continue;
					int newIndexX = indexX + j1, newIndexY = indexY + j2, newIndexZ = indexZ + j3;
					if (newIndexX < 0 || newIndexY < 0 || newIndexZ < 0 || newIndexX >= this->voxelsInDimX || newIndexY >= this->voxelsInDimY || newIndexZ >= this->voxelsInDimZ)
					{
						//printf("X:%d Y:%d Z:%d voxels in dim:%d \n", newIndexX, newIndexY, newIndexZ, this->voxelsInDim);
						continue;
					}
					this->grid[newIndexX][newIndexY][newIndexZ] = true;
				}
			}
		}
	}
	GenerateMesh(e57);
}

void Cubes::GenerateMesh(E57* e57)
{	
	for (int i1 = 0; i1 < this->voxelsInDimX; i1++)
	{
		for (int i2 = 0; i2 < this->voxelsInDimY; i2++)
		{
			for (int i3 = 0; i3 < this->voxelsInDimZ; i3++)
			{
				if (this->grid[i1][i2][i3]) 
				{
					CreateCube(i1, i2, i3, e57);					
				}
			}
		}
	}
}

void Cubes::CreateCube(int x, int y, int z, E57 * e57)
{
	Cube cube;
	float minx, miny, minz;
	if (e57)
	{
		minx = e57->getInfo().minX;
		miny = e57->getInfo().minY;
		minz = e57->getInfo().minZ;
	}
	else
	{
		minx = miny = minz = -0.5f;
	}

	cube.verteces[0] = { x * this->voxelSize, y * this->voxelSize, z * this->voxelSize };	
	cube.verteces[1] = { x * this->voxelSize + this->voxelSize, y * this->voxelSize, z * this->voxelSize };
	cube.verteces[2] = { x * this->voxelSize, y * this->voxelSize + this->voxelSize, z * this->voxelSize };
	cube.verteces[3] = { x * this->voxelSize, y * this->voxelSize, z * this->voxelSize + this->voxelSize };
	cube.verteces[4] = { x * this->voxelSize + this->voxelSize, y * this->voxelSize + this->voxelSize, z * this->voxelSize };
	cube.verteces[5] = { x * this->voxelSize, y * this->voxelSize + this->voxelSize, z * this->voxelSize + this->voxelSize };
	cube.verteces[6] = { x * this->voxelSize + this->voxelSize, y * this->voxelSize, z * this->voxelSize + this->voxelSize };
	cube.verteces[7] = { x * this->voxelSize + this->voxelSize, y * this->voxelSize + this->voxelSize, z * this->voxelSize + this->voxelSize };
	
	for (glm::vec3& ver : cube.verteces)
	{
		ver.x += minx;
		ver.y += miny;
		ver.z += minz;
	}

	//front (z-)
	if (z <= 0 || (z > 0 && !this->grid[x][y][z - 1]))
	{
		cube.sides[0].active = true;
		cube.sides[0].triangles[0] = {cube.verteces[0] , cube.verteces[1] , cube.verteces[2]};
		cube.sides[0].triangles[1] = { cube.verteces[1] , cube.verteces[4] , cube.verteces[2] };

		cube.sides[0].triangles[0].computeNormal();
		cube.sides[0].triangles[1].normal = cube.sides[0].triangles[0].normal;

		this->numOfTriangels += 2;
	}

	//leftSide (x-)
	if (x <= 0 || (x > 0 && !this->grid[x - 1][y][z]))
	{
		cube.sides[1].active = true;
		cube.sides[1].triangles[0] = { cube.verteces[0] , cube.verteces[2] , cube.verteces[3] };
		cube.sides[1].triangles[1] = { cube.verteces[2] , cube.verteces[5] , cube.verteces[3] };

		cube.sides[1].triangles[0].computeNormal();
		cube.sides[1].triangles[1].normal = cube.sides[1].triangles[0].normal;

		this->numOfTriangels += 2;
	}

	//rightSide (x+)
	if (x >= this->voxelsInDimX - 1 || (x < this->voxelsInDimX - 1 && !this->grid[x + 1][y][z]))
	{
		cube.sides[2].active = true;
		cube.sides[2].triangles[0] = { cube.verteces[1] , cube.verteces[6] , cube.verteces[4] };
		cube.sides[2].triangles[1] = { cube.verteces[4] , cube.verteces[6] , cube.verteces[7] };

		cube.sides[2].triangles[0].computeNormal();
		cube.sides[2].triangles[1].normal = cube.sides[2].triangles[0].normal;

		this->numOfTriangels += 2;
	}

	//back (z+)
	if (z >= this->voxelsInDimZ - 1 || (z < this->voxelsInDimZ - 1 && !this->grid[x][y][z + 1]))
	{
		cube.sides[3].active = true;
		cube.sides[3].triangles[0] = { cube.verteces[3] , cube.verteces[7] , cube.verteces[6] };
		cube.sides[3].triangles[1] = { cube.verteces[3] , cube.verteces[5] , cube.verteces[7] };

		cube.sides[3].triangles[0].computeNormal();
		cube.sides[3].triangles[1].normal = cube.sides[3].triangles[0].normal;

		this->numOfTriangels += 2;
	}

	//up (y+)
	if (y >= this->voxelsInDimY - 1 || (y < this->voxelsInDimY - 1 && !this->grid[x][y + 1][z]))
	{
		cube.sides[4].active = true;
		cube.sides[4].triangles[0] = { cube.verteces[2] , cube.verteces[4] , cube.verteces[5] };
		cube.sides[4].triangles[1] = { cube.verteces[4] , cube.verteces[7] , cube.verteces[5] };

		cube.sides[4].triangles[0].computeNormal();
		cube.sides[4].triangles[1].normal = cube.sides[4].triangles[0].normal;

		this->numOfTriangels += 2;
	}

	//down (y-)
	if (y <= 0 || (y > 0 && !this->grid[x][y - 1][z]))
	{
		cube.sides[5].active = true;
		cube.sides[5].triangles[0] = { cube.verteces[0] , cube.verteces[3] , cube.verteces[1] };
		cube.sides[5].triangles[1] = { cube.verteces[1] , cube.verteces[3] , cube.verteces[6] };

		cube.sides[5].triangles[0].computeNormal();
		cube.sides[5].triangles[1].normal = cube.sides[5].triangles[0].normal;

		this->numOfTriangels += 2;
	}

	this->cubes.push_back(cube);
}

std::vector<Cube>& Cubes::getCubes()
{
	return this->cubes;
}

Cubes::~Cubes()
{
}
