#include "MarchingCubes.h"

struct Help {
	float exponent, result;
};
Help maxExponent = { -1000, 0 }, minExponent = { 1000, 0 }, maxResult = { 0,  -1000 }, minResult = {0, 1000};

void MarchingCubes::InitGrid()
{
	this->grid.clear();

	NormilizedPointsInfo& info = e57->getInfo();
	this->voxelsInDimX = (int)((info.maxX - info.minX) / voxelSize) + 1;
	this->voxelsInDimY = (int)((info.maxY - info.minY) / voxelSize) + 1;
	this->voxelsInDimZ = (int)((info.maxZ - info.minZ) / voxelSize) + 1; this->voxelsInDimX = this->voxelsInDimY = this->voxelsInDimZ = (int)(1.0f / voxelSize) + 1;
	
	this->grid = std::vector<std::vector<std::vector<float>>>(this->voxelsInDimX, std::vector<std::vector<float>>(this->voxelsInDimY, std::vector<float>(this->voxelsInDimZ, 0.0f)));
}

MarchingCubes::MarchingCubes(E57* e57) : ReconstructionAlgorithm(e57)
{
	this->margin = 5;
	this->voxelSize = 0.01f;
}

MarchingCubes::MarchingCubes(float voxelSize, int margin, E57*e57) : ReconstructionAlgorithm(e57)
{

	this->margin = margin;
	this->voxelSize = voxelSize;
}

void MarchingCubes::SetVoxelSize(float voxelSize)
{
	this->voxelSize = voxelSize;
}

void MarchingCubes::SetMargin(int margin)
{
	this->margin = margin;
}

void MarchingCubes::SetIsoLevel(float isolevel)
{
	this->isolevel = isolevel;
}

void MarchingCubes::SetGrid()
{
	
	// -0.5, 0.5	
	int pointCount = e57->getPoints().size();
	int numThreads = std::thread::hardware_concurrency();
	int chunkSize = pointCount / numThreads;

	std::vector<std::thread> threads;

	// Create threads to process the points in parallel
	for (int t = 0; t < numThreads; ++t)
	{
		int startIdx = t * chunkSize;
		int endIdx = (t == numThreads - 1) ? pointCount : (startIdx + chunkSize);

		threads.push_back(std::thread(&MarchingCubes::SetGridInRange, this, startIdx, endIdx));
	}

	// Join all threads to ensure they complete before continuing
	for (auto& t : threads)
	{
		t.join();
	}	

	//printf("\nMax Exponent exp[%f] res[%f]\n", maxExponent.exponent, maxExponent.result);
	//printf("Min Exponent exp[%f] res[%f]\n", minExponent.exponent, minExponent.result);
	//printf("Max Result exp[%f] res[%f]\n", maxResult.exponent, maxResult.result);
	//printf("Min Result exp[%f] res[%f]\n\n", minResult.exponent, minResult.result);

	//printf("num of triangles: %d\n", triangles.size());
}

glm::vec3 MarchingCubes::InterpolateEdge(int x, int y, int z, int edge)
{
	glm::vec3 cornerPositions[8] = {
		glm::vec3(x, y, z),
		glm::vec3(x + 1, y, z),
		glm::vec3(x + 1, y + 1, z),
		glm::vec3(x, y + 1, z),
		glm::vec3(x, y, z + 1),
		glm::vec3(x + 1, y, z + 1),
		glm::vec3(x + 1, y + 1, z + 1),
		glm::vec3(x, y + 1, z + 1)
	};

	int edgeVertices[12][2] = {
		{0, 1}, {1, 2}, {2, 3}, {3, 0},
		{4, 5}, {5, 6}, {6, 7}, {7, 4},
		{0, 4}, {1, 5}, {2, 6}, {3, 7}
	};

	glm::vec3 p1 = cornerPositions[edgeVertices[edge][0]];
	glm::vec3 p2 = cornerPositions[edgeVertices[edge][1]];

	int ix1 = static_cast<int>(p1.x);
	int iy1 = static_cast<int>(p1.y);
	int iz1 = static_cast<int>(p1.z);

	int ix2 = static_cast<int>(p2.x);
	int iy2 = static_cast<int>(p2.y);
	int iz2 = static_cast<int>(p2.z);

	float val1 = this->grid[ix1][iy1][iz1];
	float val2 = this->grid[ix2][iy2][iz2];
	
	glm::vec3 interpolatedVertex;
	
	if (fabs(isolevel - val1) < 0.00001)
		interpolatedVertex = p1;
	else if (fabs(isolevel - val2) < 0.00001)
		interpolatedVertex = p2 ;
	else if (fabs(val1 - val2) < 0.00001)
		interpolatedVertex = p1;
	else
	{
		float mu = (isolevel - val1) / (val2 - val1);
		interpolatedVertex.x = p1.x + mu * (p2.x - p1.x);
		interpolatedVertex.y = p1.y + mu * (p2.y - p1.y);
		interpolatedVertex.z = p1.z + mu * (p2.z - p1.z);
	}
	interpolatedVertex *= this->voxelSize;
	if (e57)
	{
		interpolatedVertex.x += e57->getInfo().minX;
		interpolatedVertex.y += e57->getInfo().minY;
		interpolatedVertex.z += e57->getInfo().minZ;
	}
	else
		interpolatedVertex -= glm::vec3(0.5f);

	return interpolatedVertex;
}

void MarchingCubes::CreateTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
	// Create triangle with three vertices
	if (a.x == -1 || b.x == -1 || c.x == -1)
		return;
	Triangle tri;
	tri.a.position = a;
	tri.b.position = b;
	tri.c.position = c;
	tri.computeNormal();

	std::unique_lock<std::mutex> lock(this->trianglesMutex);

	this->getTriangles().push_back(tri);

	lock.unlock();
}

float MarchingCubes::CalculateDensity(glm::vec3 point, glm::vec3 min, glm::vec3 index)
{
	float voxelCenterX = (index.x * voxelSize + min.x) + voxelSize / 2;
	float voxelCenterY = (index.y * voxelSize + min.y) + voxelSize / 2;
	float voxelCenterZ = (index.z * voxelSize + min.z) + voxelSize / 2;

	float dx = point.x - voxelCenterX;
	float dy = point.y - voxelCenterY;
	float dz = point.z - voxelCenterZ;

	float distanceSquared = dx * dx + dy * dy + dz * dz;
	float sigma = voxelSize * 0.5;
	float sigmaSquared = sigma * sigma;

	float exponent = -distanceSquared / (2.0f * sigmaSquared);
	float result = exp(exponent);

	/*if (exponent > maxExponent.exponent)
	{
		maxExponent.exponent = exponent;
		maxExponent.result = result;
	}
	if (result > minResult.result)
	{
		maxResult.exponent = exponent;
		maxResult.result = result;
	}
	if (exponent < minExponent.exponent)
	{
		minExponent.exponent = exponent;
		minExponent.result = result;
	}
	if (result < minResult.result)
	{
		minResult.exponent = exponent;
		minResult.result = result;
	}*/

	return result;
}

void MarchingCubes::GenerateMesh()
{
	/*for (int i1 = 0; i1 < this->voxelsInDimX; ++i1)
	{
		for (int i2 = 0; i2 < this->voxelsInDimY; ++i2)
		{
			for (int i3 = 0; i3 < this->voxelsInDimZ; ++i3)
			{
				if (this->grid[i1][i2][i3])
				{
					GenerateCubeMesh(i1, i2, i3);
				}
			}
		}
	}*/

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
				threads.push_back(std::thread(&MarchingCubes::GenerateMeshInRange, this, startX, endX, startY, endY, startZ, endZ));
			}
		}
	}

	// Join all threads to ensure they complete before continuing
	for (auto& t : threads)
	{
		t.join();
	}
}
int allEdges = 0;
void MarchingCubes::GenerateCubeMesh(int x, int y, int z)
{
	if (x + 1 >= this->voxelsInDimX || y + 1 >= this->voxelsInDimY || z + 1 >= this->voxelsInDimZ) 
		return;
	// Determine the corner indices based on the scalar field values (this could be density, signed distance, etc.)
	int cubeIndex = 0;
	if (this->grid[x][y][z] >= isolevel) cubeIndex |= 1;
	if (this->grid[x + 1][y][z] >= isolevel) cubeIndex |= 2;
	if (this->grid[x + 1][y + 1][z] >= isolevel) cubeIndex |= 4;
	if (this->grid[x][y + 1][z] >= isolevel) cubeIndex |= 8;
	if (this->grid[x][y][z + 1] >= isolevel) cubeIndex |= 16;
	if (this->grid[x + 1][y][z + 1] >= isolevel) cubeIndex |= 32;
	if (this->grid[x + 1][y + 1][z + 1] >= isolevel) cubeIndex |= 64;
	if (this->grid[x][y + 1][z + 1] >= isolevel) cubeIndex |= 128;
		

	// Get the edges that are intersected by the surface
	int edges = edgeTable[cubeIndex];

	if (edges == 0) return; // No intersection

	int intersected = 0;
	// Interpolate the surface at the edges and create triangles
	glm::vec3 vertices[12];
	for (int i = 0; i < 12; i++) {
		if (edges & (1 << i)) {
			vertices[i] = InterpolateEdge(x, y, z, i); // Interpolate edge i
			++intersected;
		}
	}
	allEdges += intersected;
	//printf("intersected: %d all %d\n", intersected, allEdges);

	// Generate triangles from the interpolated vertices
	for (int i = 0; triTable[cubeIndex][i] != -1; i += 3) {
		/*if (triTable[cubeIndex][i] >= 12 || triTable[cubeIndex][i + 1] >= 12 || triTable[cubeIndex][i + 2] >= 12)
		{
			printf("Out of bounds: cubeIndex=%d, i=%d, vertices.size()=%zu\n", cubeIndex, i, 12);
			continue;
		}*/
		CreateTriangle(
			vertices[triTable[cubeIndex][i]],
			vertices[triTable[cubeIndex][i + 1]],
			vertices[triTable[cubeIndex][i + 2]]
		);
	}
}

void MarchingCubes::SetGridInRange(int startIdx, int endIdx)
{
	minX = e57->getInfo().minX;
	minY = e57->getInfo().minY;
	minZ = e57->getInfo().minZ;

	std::vector<E57Point>& points = e57->getPoints();

	for (int i = startIdx; i < endIdx; i++)
	{
		float x = points[i].position.x, y = points[i].position.y, z = points[i].position.z;

		int indexX = (x - minX) / voxelSize;
		int indexY = (y - minY) / voxelSize;
		int indexZ = (z - minZ) / voxelSize;

		float density = CalculateDensity(glm::vec3(x, y, z), glm::vec3(minX, minY, minZ), glm::vec3(indexX, indexY, indexZ));

		grid[indexX][indexY][indexZ] = std::max(grid[indexX][indexY][indexZ], density);

		for (int j1 = -margin; j1 <= margin; ++j1)
		{
			for (int j2 = -margin; j2 <= margin; ++j2)
			{
				for (int j3 = -margin; j3 <= margin; ++j3)
				{
					if ((j1 == 0 && j2 == 0 && j3 == 0) || (abs(j1) + abs(j2) + abs(j3) > margin))
						continue;

					int newIndexX = indexX + j1, newIndexY = indexY + j2, newIndexZ = indexZ + j3;
					if (newIndexX < 0 || newIndexY < 0 || newIndexZ < 0 || newIndexX >= voxelsInDimX || newIndexY >= voxelsInDimY || newIndexZ >= voxelsInDimZ)
					{
						continue;
					}

					float marginDensity = CalculateDensity(glm::vec3(x, y, z), glm::vec3(minX, minY, minZ), glm::vec3(newIndexX, newIndexY, newIndexZ));
					grid[newIndexX][newIndexY][newIndexZ] = std::max(grid[newIndexX][newIndexY][newIndexZ], marginDensity);
				}
			}
		}
	}
}

void MarchingCubes::GenerateMeshInRange(int startX, int endX, int startY, int endY, int startZ, int endZ)
{
	for (int i1 = startX; i1 < endX; ++i1)
	{
		for (int i2 = startY; i2 < endY; ++i2)
		{
			for (int i3 = startZ; i3 < endZ; ++i3)
			{
				if (this->grid[i1][i2][i3])
				{
					GenerateCubeMesh(i1, i2, i3);
				}
			}
		}
	}
}

void MarchingCubes::Run()
{
	this->getTriangles().clear();
	GenerateMesh();
}

void MarchingCubes::SetUp()
{
	InitGrid();
	SetGrid();
}

MarchingCubes::~MarchingCubes()
{
}
