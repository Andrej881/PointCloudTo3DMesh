#include "MarchingCubes.h"

struct Help {
	float exponent, result;
};
Help maxExponent = { -1000, 0 }, minExponent = { 1000, 0 }, maxResult = { 0,  -1000 }, minResult = {0, 1000};

void MarchingCubes::InitGrid()
{
	this->grid.clear();

	NormilizedPointsInfo& info = e57->getInfo();
	this->voxelsInDimX = (int)((info.maxX - info.minX) / voxelSize) + 2;
	this->voxelsInDimY = (int)((info.maxY - info.minY) / voxelSize) + 2;
	this->voxelsInDimZ = (int)((info.maxZ - info.minZ) / voxelSize) + 2;
	
	this->grid = std::vector<std::vector<std::vector<float>>>(this->voxelsInDimX, std::vector<std::vector<float>>(this->voxelsInDimY, std::vector<float>(this->voxelsInDimZ,-FLT_MAX)));
}

MarchingCubes::MarchingCubes(E57* e57) : ReconstructionAlgorithm(e57)
{
	this->voxelSize = 0.01f;
	this->isolevel = 0.2f;
	this->margin = 1;
	this->sigmaMultiplier = 0.75f; //multiplies voxelSize
}

MarchingCubes::MarchingCubes(float isolevel, float voxelSize,int margin, float sigmaMulti, E57* e57) : ReconstructionAlgorithm(e57)
{
	this->sigmaMultiplier = sigmaMulti;
	this->margin = margin;
	this->voxelSize = voxelSize;
	this->isolevel = isolevel;
}

void MarchingCubes::SetVoxelSize(float voxelSize)
{
	this->voxelSize = voxelSize;
}

void MarchingCubes::SetIsoLevel(float isolevel)
{
	this->isolevel = isolevel;
}

void MarchingCubes::SetMargin(int margin)
{
	this->margin = margin;
}

void MarchingCubes::SetSigmaMultiplier(float sigmaMultiplier)
{
	this->sigmaMultiplier = sigmaMultiplier;
}

int MarchingCubes::GetVoxelsInDimX()
{
	return this->voxelsInDimX;
}
int MarchingCubes::GetVoxelsInDimY()
{
	return this->voxelsInDimY;
}
int MarchingCubes::GetVoxelsInDimZ()
{
	return this->voxelsInDimZ;
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


}

glm::vec3 MarchingCubes::InterpolateEdge(int x, int y, int z, int edge)
{
	static const int edgeTable[12][2] = {
		{0, 1}, {1, 3}, {3, 2}, {2, 0},
		{4, 5}, {5, 7}, {7, 6}, {6, 4},
		{0, 4}, {1, 5}, {3, 7}, {2, 6}
	};

	glm::ivec3 offsets[8] = {
		{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0},
		{0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}
	};

	int v1 = edgeTable[edge][0];
	int v2 = edgeTable[edge][1];

	glm::ivec3 p1 = glm::ivec3(x, y, z) + offsets[v1];
	glm::ivec3 p2 = glm::ivec3(x, y, z) + offsets[v2];

	double d1 = grid[p1.x][p1.y][p1.z];
	double d2 = grid[p2.x][p2.y][p2.z];

	if (std::abs(d1 - d2) < 1e-6) return glm::vec3(p1);

	double t = (isolevel - d1) / (d2 - d1);

	auto interpolatedVertex = glm::mix(glm::vec3(p1), glm::vec3(p2), t);

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

	std::unique_lock<std::mutex> lock(triangleMutex);

	this->GetTriangles().push_back(tri);

	lock.unlock();
}

float MarchingCubes::CalculateDensity(glm::vec3 point, glm::vec3 min, glm::vec3 index)
{
	float voxelX = (index.x * voxelSize + min.x) + voxelSize / 2;
	float voxelY = (index.y * voxelSize + min.y) + voxelSize / 2;
	float voxelZ = (index.z * voxelSize + min.z) + voxelSize / 2;

	float distanceSquared = glm::distance2(point, glm::vec3(voxelX, voxelY, voxelZ));
	float sigma = voxelSize * this->sigmaMultiplier;
	float sigmaSquared = sigma * sigma;

	float exponent = -distanceSquared / (2.0f * sigmaSquared);
	float result = exp(exponent);
	
	return result;
}

void MarchingCubes::GenerateMesh()
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
void MarchingCubes::GenerateCubeMesh(int x, int y, int z)
{
	if (x + 1 >= this->voxelsInDimX || y + 1 >= this->voxelsInDimY || z + 1 >= this->voxelsInDimZ) 
		return;
	// Determine the corner indices based on the scalar field values (this could be density, signed distance, etc.)
	int cubeIndex = 0;
	if (this->grid[x][y][z] < isolevel) cubeIndex |= 1;
	if (this->grid[x + 1][y][z] < isolevel) cubeIndex |= 2;
	if (this->grid[x + 1][y + 1][z] < isolevel) cubeIndex |= 4;
	if (this->grid[x][y + 1][z] < isolevel) cubeIndex |= 8;
	if (this->grid[x][y][z + 1] < isolevel) cubeIndex |= 16;
	if (this->grid[x + 1][y][z + 1] < isolevel) cubeIndex |= 32;
	if (this->grid[x + 1][y + 1][z + 1] < isolevel) cubeIndex |= 64;
	if (this->grid[x][y + 1][z + 1] < isolevel) cubeIndex |= 128;
		
	
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

	// Generate triangles from the interpolated vertices
	for (int i = 0; triTable[cubeIndex][i] != -1; i += 3) {
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

		for (int dx = -margin; dx <= margin; dx++)
			for (int dy = -margin; dy <= margin; dy++)
				for (int dz = -margin; dz <= margin; dz++)
				{
					int ix = indexX + dx;
					int iy = indexY + dy;
					int iz = indexZ + dz;

					if (ix < 0 || iy < 0 || iz < 0 || ix >= this->voxelsInDimX || iy >= this->voxelsInDimY || iz >= this->voxelsInDimZ)
						continue;
					float density = CalculateDensity(glm::vec3(x, y, z), glm::vec3(minX, minY, minZ), glm::vec3(ix, iy, iz));
					std::unique_lock<std::mutex> lock(this->trianglesMutex);
					if (-FLT_MAX == grid[ix][iy][iz])
						grid[ix][iy][iz] = density;
					else
						grid[ix][iy][iz] += density;
					lock.unlock();
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
				if (this->stopEarly)
					return;
				GenerateCubeMesh(i1, i2, i3);
			}
		}
	}
}

void MarchingCubes::Run()
{
	auto start = std::chrono::high_resolution_clock::now();

	this->running = true;
	std::unique_lock<std::mutex> lock(triangleMutex);
	this->GetTriangles().clear();
	lock.unlock();
	GenerateMesh();
	this->running = this->stopEarly = false;

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = end - start;

	std::unique_lock<std::mutex> lock2(triangleMutex);
	std::cout << "Trvanie algoritmu: " << duration.count() << " sec. a trojuholnikov " << this->GetTriangles().size() << std::endl;
	lock2.unlock();
}

void MarchingCubes::SetUp()
{
	InitGrid();
	SetGrid();
}

MarchingCubes::~MarchingCubes()
{
}
