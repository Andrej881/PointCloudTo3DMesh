#pragma once
#include <unordered_set>

#include "KDTree.h"
#include "ReconstructionAlgorithm.h"


struct Triangle2 {
	KDTreeNode* p1;
	KDTreeNode* p2;
	KDTreeNode* p3;

	Triangle triangle;
	Triangle2(KDTreeNode* p1, KDTreeNode* p2, KDTreeNode* p3) {
		this->p1 = p1;
		this->p2 = p2;
		this->p3 = p3;
		if (p1 != nullptr && p2 != nullptr && p3 != nullptr)
		{
			triangle.a = *p1->point;
			triangle.b = *p2->point;
			triangle.c = *p3->point;
			triangle.computeNormal();

			glm::vec3 triangleCenter = (triangle.a.position + triangle.b.position + triangle.c.position) / 3.0f;
			glm::vec3 objectCenter(0);
			glm::vec3 toOutside = triangleCenter - objectCenter;
			if (glm::dot(triangle.normal, toOutside) < 0.0f)
				triangle.normal = -triangle.normal;
		}
	}
};

class BallPivoting : public ReconstructionAlgorithm
{
private:
	float radius;
	KDTree* tree;

	Triangle2 FindInitialTriangle();
	bool IsTriangleValid(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c);

public:
	BallPivoting(E57* e57);
	BallPivoting(E57* e57, float radius);

	void SetRadius(float radius);

	void Run() override;
	void SetUp() override;
	~BallPivoting();
};

