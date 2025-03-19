#pragma once

#include "E57.h"
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

class Cubes
{
private:
	float voxelSize;
	int margin;

	int voxelsInDimX, voxelsInDimY, voxelsInDimZ;
	std::vector<std::vector<std::vector<bool>>> grid;
	std::vector<Cube> cubes;

	void GenerateMesh(E57* e57);
	void CreateCube(int x, int y, int z, E57* e57);
public:
	void InitGrid(E57* e57);
	Cubes(float voxelSize, int margin, E57& e57);
	Cubes(float voxelSize, int margin, std::vector<float>& points);
	void SetGrid(std::vector<float>& points, E57* e57);
	std::vector<Cube>& getCubes();
	~Cubes();
	int numOfTriangels = 0;
};

