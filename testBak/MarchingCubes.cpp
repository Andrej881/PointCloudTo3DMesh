#include "MarchingCubes.h"

void MarchingCubes::InitGrid(std::vector<float>& points, E57* e57)
{
	this->triangles.clear();
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
	this->grid = std::vector<std::vector<std::vector<float>>>(this->voxelsInDimX, std::vector<std::vector<float>>(this->voxelsInDimY, std::vector<float>(this->voxelsInDimZ, 0.0f)));
}

MarchingCubes::MarchingCubes(float voxelSize, int margin, E57& e57)
{
	this->margin = margin;
	this->voxelSize = voxelSize;

	InitGrid(e57.getPoints(), &e57);
	SetGrid(e57.getPoints(), &e57);
}

MarchingCubes::MarchingCubes(float voxelSize, int margin, std::vector<float>& points)
{
	this->margin = margin;
	this->voxelSize = voxelSize;

	InitGrid(points, nullptr);
	SetGrid(points, nullptr);
}

void MarchingCubes::SetGrid(std::vector<float>& points, E57* e57)
{
	// -0.5, 0.5
	for (int i = 0; i < points.size(); i = i + 3) {
		//printf("[%d] / [%d]\n", i, points.size());
		float x = points[i], y = points[i + 1], z = points[i + 2];
		
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

		float distance = sqrt((x - minX) * (x - minX) + (y - minY) * (y - minY) + (z - minZ) * (z - minZ));
		float density = CalculateDensity(distance, voxelSize);

		this->grid[indexX][indexY][indexZ] = std::max(this->grid[indexX][indexY][indexZ], density);
		

		for (int j1 = -margin; j1 <= margin; ++j1)
		{
			for (int j2 = -margin; j2 <= margin; ++j2)
			{
				for (int j3 = -margin; j3 <= margin; ++j3)
				{
					//if ((j1 == 0 && j2 == 0 && j3 == 0) || (abs(j1) + abs(j2) + abs(j3) != margin))
						//continue;
					int newIndexX = indexX + j1, newIndexY = indexY + j2, newIndexZ = indexZ + j3;
					if (newIndexX < 0 || newIndexY < 0 || newIndexZ < 0 || newIndexX >= this->voxelsInDimX || newIndexY >= this->voxelsInDimY || newIndexZ >= this->voxelsInDimZ)
					{
						//printf("X:%d Y:%d Z:%d voxels in dim:%d \n", newIndexX, newIndexY, newIndexZ, this->voxelsInDim);
						continue;
					}
					float newDistance = sqrt(j1 * j1 + j2 * j2 + j3 * j3) * voxelSize;
					float newDensity = CalculateDensity(newDistance, voxelSize);
					this->grid[newIndexX][newIndexY][newIndexZ] = std::max(this->grid[newIndexX][newIndexY][newIndexZ], newDensity);
					
				}
			}
		}
	}

	GenerateMesh();
	printf("num of triangles: %d\n", triangles.size());
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

	if (ix1 < 0 || ix1 >= this->voxelsInDimX || iy1 < 0 || iy1 >= this->voxelsInDimY || iz1 < 0 || iz1 >= this->voxelsInDimZ ||
		ix2 < 0 || ix2 >= this->voxelsInDimX || iy2 < 0 || iy2 >= this->voxelsInDimY || iz2 < 0 || iz2 >= this->voxelsInDimZ) {
		printf("Indices out of bounds\n");
		return glm::vec3(-1);
	}

	float val1 = this->grid[ix1][iy1][iz1];
	float val2 = this->grid[ix2][iy2][iz2];
	
	glm::vec3 interpolatedVertex;
	
	if (fabs(isolevel - val1) < 0.00001)
		return p1 * this->voxelSize;
	if (fabs(isolevel - val2) < 0.00001)
		return p2 * this->voxelSize;
	if (fabs(val1 - val2) < 0.00001)
		return p1 * this->voxelSize;
	float mu = (isolevel - val1) / (val2 - val1);
	interpolatedVertex.x = p1.x + mu * (p2.x - p1.x);
	interpolatedVertex.y = p1.y + mu * (p2.y - p1.y);
	interpolatedVertex.z = p1.z + mu * (p2.z - p1.z);

	return interpolatedVertex * this->voxelSize;
}

void MarchingCubes::CreateTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
	// Create triangle with three vertices
	if (a.x == -1 || b.x == -1 || c.x == -1)
		return;
	Triangle tri;
	tri.a = a;
	tri.b = b;
	tri.c = c;
	tri.computeNormal();
	this->triangles.push_back(tri);
}

std::vector<Triangle>& MarchingCubes::getTriangles()
{
	return this->triangles;
}

float MarchingCubes::CalculateDensity(float distance, float sigma)
{
	// Using a Gaussian function for smoother density distribution
	return exp(-0.5 * (distance * distance) / (sigma * sigma));
}

void MarchingCubes::GenerateMesh()
{
	for (int i1 = 0; i1 < this->voxelsInDimX; i1++)
	{
		for (int i2 = 0; i2 < this->voxelsInDimY; i2++)
		{
			for (int i3 = 0; i3 < this->voxelsInDimZ; i3++)
			{
				if (this->grid[i1][i2][i3])
				{
					GenerateCubeMesh(i1, i2, i3);
				}
			}
		}
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
		if (triTable[cubeIndex][i] >= 12 || triTable[cubeIndex][i + 1] >= 12 || triTable[cubeIndex][i + 2] >= 12)
		{
			printf("Out of bounds: cubeIndex=%d, i=%d, vertices.size()=%zu\n", cubeIndex, i, 12);
			continue;
		}
		CreateTriangle(
			vertices[triTable[cubeIndex][i]],
			vertices[triTable[cubeIndex][i + 1]],
			vertices[triTable[cubeIndex][i + 2]]
		);
	}
}

MarchingCubes::~MarchingCubes()
{
}