#include "Cubes.h"

void Cubes::InitGrid()
{
	this->grid.clear();

	NormilizedPointsInfo& info = e57->getInfo();
	this->voxelsInDimX = (int)((info.maxX - info.minX) / voxelSize) + 1;
	this->voxelsInDimY = (int)((info.maxY - info.minY) / voxelSize) + 1;
	this->voxelsInDimZ = (int)((info.maxZ - info.minZ) / voxelSize) + 1;

	this->grid = std::vector<std::vector<std::vector<bool>>>(this->voxelsInDimX, std::vector<std::vector<bool>>(this->voxelsInDimY, std::vector<bool>(this->voxelsInDimZ, false)));
}

Cubes::Cubes(E57* e57) : ReconstructionAlgorithm(e57)
{
	this->margin = 1;
	this->voxelSize = 0.01f;
}

Cubes::Cubes(float voxelSize, int margin, E57* e57) : ReconstructionAlgorithm(e57)
{
	this->margin = margin;
	this->voxelSize = voxelSize;
}

void Cubes::SetVoxelSize(float voxelSize)
{
	this->voxelSize = voxelSize;
}

void Cubes::SetMargin(int margin)
{
	this->margin = margin;
}

void Cubes::SetGrid()
{
	int pointCount = e57->getPoints().size();
	int numThreads = std::thread::hardware_concurrency();
	int chunkSize = pointCount / numThreads;

	std::vector<std::thread> threads;

	// Create threads to process the points in parallel
	for (int t = 0; t < numThreads; ++t)
	{
		int startIdx = t * chunkSize;
		int endIdx = (t == numThreads - 1) ? pointCount : (startIdx + chunkSize);

		threads.push_back(std::thread(&Cubes::SetGridInRange, this, startIdx, endIdx));
	}
	// Join all threads to ensure they complete before continuing
	for (auto& t : threads)
	{
		t.join();
	}

}

void Cubes::SetGridInRange(int startIdx, int endIdx)
{
	std::vector<E57Point>& points = e57->getPoints();

	minX = e57->getInfo().minX;
	minY = e57->getInfo().minY;
	minZ = e57->getInfo().minZ;

	for (int i = startIdx; i < endIdx; i++) {
		if (this->stopEarly)
			return;
		//printf("[%d] / [%d]\n", i, points.size());
		float x = points[i].position.x, y = points[i].position.y, z = points[i].position.z;

		int indexX = (x - minX) / voxelSize, indexY = (y - minY) / voxelSize, indexZ = (z - minZ) / voxelSize;

		if (indexX < 0 || indexY < 0 || indexZ < 0 || indexX >= this->voxelsInDimX || indexY >= this->voxelsInDimY || indexZ >= this->voxelsInDimZ)
		{
			//printf("X:%d Y:%d Z:%d voxels in dim:%d \n", indexX, indexY, indexZ, this->voxelsInDim);
			continue;
		}

		if (margin == 0)
		{

			std::unique_lock<std::mutex> lock(this->triangleMutex);
			this->grid[indexX][indexY][indexZ] = true;
			lock.unlock();
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

					std::unique_lock<std::mutex> lock(this->triangleMutex);
					this->grid[newIndexX][newIndexY][newIndexZ] = true;
					lock.unlock();
				}
			}
		}
	}
}

void Cubes::GenerateMeshInRange(int startX, int endX, int startY, int endY, int startZ, int endZ)
{
	for (int i1 = startX; i1 < endX; ++i1)
	{
		for (int i2 = startY; i2 < endY; ++i2)
		{
			for (int i3 = startZ; i3 < endZ; ++i3)
			{
				if (this->stopEarly)
					return;
				if (this->grid[i1][i2][i3])
				{
					CreateCube(i1, i2, i3);
				}
			}
		}
	}
}

void Cubes::GenerateMesh()
{	
	int numThreads = std::thread::hardware_concurrency();
	int chunkSizeX = voxelsInDimX / numThreads;
	int chunkSizeY = voxelsInDimY / numThreads;
	int chunkSizeZ = voxelsInDimZ / numThreads;

	std::vector<std::thread> threads;

	// Create threads to process different chunks of the 3D grid
	for (int tX = 0; tX < numThreads; ++tX)
	{
		for (int tY = 0; tY < numThreads; ++tY)
		{
			for (int tZ = 0; tZ < numThreads; ++tZ)
			{
				int startX = tX * chunkSizeX;
				int endX = (tX == numThreads - 1) ? voxelsInDimX : startX + chunkSizeX;

				int startY = tY * chunkSizeY;
				int endY = (tY == numThreads - 1) ? voxelsInDimY : startY + chunkSizeY;

				int startZ = tZ * chunkSizeZ;
				int endZ = (tZ == numThreads - 1) ? voxelsInDimZ : startZ + chunkSizeZ;

				// Launch a thread for each chunk
				threads.push_back(std::thread(&Cubes::GenerateMeshInRange, this, startX, endX, startY, endY, startZ, endZ));
			}
		}
	}

	// Join all threads to ensure they complete before continuing
	for (auto& t : threads)
	{
		t.join();
	}
}

void Cubes::CreateCube(int x, int y, int z)
{
	Cube cube;

	cube.verteces[0] = { glm::vec3(x * this->voxelSize, y * this->voxelSize, z * this->voxelSize) };	
	cube.verteces[1] = { glm::vec3(x * this->voxelSize + this->voxelSize, y * this->voxelSize, z * this->voxelSize) };
	cube.verteces[2] = { glm::vec3(x * this->voxelSize, y * this->voxelSize + this->voxelSize, z * this->voxelSize) };
	cube.verteces[3] = { glm::vec3(x * this->voxelSize, y * this->voxelSize, z * this->voxelSize + this->voxelSize) };
	cube.verteces[4] = { glm::vec3(x * this->voxelSize + this->voxelSize, y * this->voxelSize + this->voxelSize, z * this->voxelSize) };
	cube.verteces[5] = { glm::vec3(x * this->voxelSize, y * this->voxelSize + this->voxelSize, z * this->voxelSize + this->voxelSize) };
	cube.verteces[6] = { glm::vec3(x * this->voxelSize + this->voxelSize, y * this->voxelSize, z * this->voxelSize + this->voxelSize) };
	cube.verteces[7] = { glm::vec3(x * this->voxelSize + this->voxelSize, y * this->voxelSize + this->voxelSize, z * this->voxelSize + this->voxelSize) };
	
	for (E57Point& ver : cube.verteces)
	{
		ver.position.x += minX;
		ver.position.y += minY;
		ver.position.z += minZ;
	}

	//front (z-)
	if (z <= 0 || (z > 0 && !this->grid[x][y][z - 1]))
	{
		cube.sides[0].active = true;
		cube.sides[0].triangles[0] = {cube.verteces[0] , cube.verteces[1] , cube.verteces[2]};
		cube.sides[0].triangles[1] = { cube.verteces[1] , cube.verteces[4] , cube.verteces[2] };

		cube.sides[0].triangles[0].computeNormal();
		cube.sides[0].triangles[1].normal = cube.sides[0].triangles[0].normal;

	}

	//leftSide (x-)
	if (x <= 0 || (x > 0 && !this->grid[x - 1][y][z]))
	{
		cube.sides[1].active = true;
		cube.sides[1].triangles[0] = { cube.verteces[0] , cube.verteces[3] , cube.verteces[2]  };
		cube.sides[1].triangles[1] = { cube.verteces[2] , cube.verteces[3], cube.verteces[5]  };

		cube.sides[1].triangles[0].computeNormal();
		cube.sides[1].triangles[1].normal = cube.sides[1].triangles[0].normal;

	}

	//rightSide (x+)
	if (x >= this->voxelsInDimX - 1 || (x < this->voxelsInDimX - 1 && !this->grid[x + 1][y][z]))
	{
		cube.sides[2].active = true;
		cube.sides[2].triangles[0] = { cube.verteces[1], cube.verteces[4] , cube.verteces[6]  };
		cube.sides[2].triangles[1] = { cube.verteces[4] , cube.verteces[7], cube.verteces[6]  };

		cube.sides[2].triangles[0].computeNormal();
		cube.sides[2].triangles[1].normal = cube.sides[2].triangles[0].normal;

	}

	//back (z+)
	if (z >= this->voxelsInDimZ - 1 || (z < this->voxelsInDimZ - 1 && !this->grid[x][y][z + 1]))
	{
		cube.sides[3].active = true;
		cube.sides[3].triangles[0] = { cube.verteces[3] , cube.verteces[6], cube.verteces[7]  };
		cube.sides[3].triangles[1] = { cube.verteces[3] , cube.verteces[7], cube.verteces[5]  };

		cube.sides[3].triangles[0].computeNormal();
		cube.sides[3].triangles[1].normal = cube.sides[3].triangles[0].normal;

	}

	//up (y+)
	if (y >= this->voxelsInDimY - 1 || (y < this->voxelsInDimY - 1 && !this->grid[x][y + 1][z]))
	{
		cube.sides[4].active = true;
		cube.sides[4].triangles[0] = { cube.verteces[2] , cube.verteces[5], cube.verteces[4]  };
		cube.sides[4].triangles[1] = { cube.verteces[4] , cube.verteces[5], cube.verteces[7]  };

		cube.sides[4].triangles[0].computeNormal();
		cube.sides[4].triangles[1].normal = cube.sides[4].triangles[0].normal;
	}

	//down (y-)
	if (y <= 0 || (y > 0 && !this->grid[x][y - 1][z]))
	{
		cube.sides[5].active = true;
		cube.sides[5].triangles[0] = { cube.verteces[0] , cube.verteces[1], cube.verteces[3]  };
		cube.sides[5].triangles[1] = { cube.verteces[1] , cube.verteces[6], cube.verteces[3]  };

		cube.sides[5].triangles[0].computeNormal();
		cube.sides[5].triangles[1].normal = cube.sides[5].triangles[0].normal;
	}
	
	for (Side side : cube.sides)
	{
		for (Triangle triangle : side.triangles)
		{
			std::unique_lock<std::mutex> lock(triangleMutex);
			this->GetTriangles().push_back(triangle);
			lock.unlock();
		}
	}
}

void Cubes::Run()
{
	auto start = std::chrono::high_resolution_clock::now();
	this->running = true;	

	SetGrid();
	if (this->stopEarly)
	{
		this->running = this->stopEarly = false;
		return;
	}

	GenerateMesh();
	this->running = this->stopEarly = false;

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = end - start;

	std::unique_lock<std::mutex> lock(triangleMutex);
	std::cout << "Trvanie algoritmu: " << duration.count() << " sec. a trojuholnikov " << this->GetTriangles().size() << std::endl;
	lock.unlock();
}

void Cubes::SetUp()
{
	std::unique_lock<std::mutex> lock(triangleMutex);
	this->GetTriangles().clear();
	lock.unlock();
	InitGrid();
}

Cubes::~Cubes()
{
}
