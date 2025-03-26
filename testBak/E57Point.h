#pragma once
#include <glm/glm.hpp>

struct E57Point {
	glm::vec3 position;
	glm::vec3 normal = glm::vec3(0.0f);
	bool hasNormal = false;
	//glm::vec3 color; later
	bool operator==(const E57Point& other)
	{
		if (this->hasNormal == other.hasNormal)
		{
			if (this->hasNormal == true)
			{
				return this->position == other.position && this->normal == other.normal;
			}
			else
			{
				return this->position == other.position;
			}
		}
		else
			return false;
	}
};