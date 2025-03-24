#pragma once

#include <glm/glm.hpp>
#include "E57.h"

struct Triangle
{
	E57Point a;
	E57Point b;
	E57Point c;

	glm::vec3 normal;

	void computeNormal() {
		if (a.hasNormal) {

			normal.x = (a.normal.x + b.normal.x + c.normal.x) / 3.0f;
			normal.y = (a.normal.y + b.normal.y + c.normal.y) / 3.0f;
			normal.z = (a.normal.z + b.normal.z + c.normal.z) / 3.0f;

			return;
		}
		auto u = b.position - a.position;
		auto v = c.position - a.position;
		normal = -glm::normalize(glm::cross(u, v));		
	}
};