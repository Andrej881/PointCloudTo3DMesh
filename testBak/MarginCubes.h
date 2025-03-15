#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Triangle
{
	glm::vec3 a, b, c;
	glm::vec3 normal;

	void computeNormal() {
		auto u = b - a;
		auto v = c - a;
		normal = -glm::normalize(glm::cross(u, v));
		//normal = glm::vec3(0, 0, 0);
	}
};

struct Side {
	bool active = false;
	Triangle triangles[2];
};

struct Cube
{
	glm::vec3 verteces[8];

	Side sides[6]; // front, leftSide, rightSide, back, up, down
	
	//tmp
	Triangle triangles[12];
};

class MarginCubes
{
private:
	float voxelSize;
	int margin;

	int voxelsInDim;
	std::vector<std::vector<std::vector<bool>>> grid;
	std::vector<Cube> cubes;

	void GenerateMesh();
	void CreateCube(int x, int y, int z);	
public:
	MarginCubes(float voxelSize, int margin, std::vector<float>& points);
	void SetGrid(std::vector<float>& points);
	std::vector<Cube>& getCubes();
	~MarginCubes();
	int numOfTriangels = 0;
};

