#pragma once
#include <glm/glm.hpp>

struct E57Point {
	glm::vec3 position;
	glm::vec3 normal = glm::vec3(0.0f);
	bool hasNormal = false;
	//glm::vec3 color; later
};