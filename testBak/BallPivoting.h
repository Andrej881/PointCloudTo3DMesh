#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <unordered_set>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/projection.hpp>
#include <functional>
#include "KDTree.h"
#include "ReconstructionAlgorithm.h"

struct Edge {
	KDTreeNode* a;
	KDTreeNode* b;

	Edge(KDTreeNode* a, KDTreeNode* b) 
	{
		this->a = a;
		this->b = b;
	}

	bool operator==(const Edge& other) const {
		return (a == other.a && b == other.b) || (a == other.b && b == other.a);
	}
};

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
	// Track processed points
	float radius, initRadius, lowerRadius, tolerance; 

	KDTree* tree;

	Triangle2 FindInitialTriangle(std::unordered_set<KDTreeNode*>& visited, KDTree& visitedTree);
	bool IsCenterOfTriangleValid(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c);
	bool IsTriangleValid(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c, KDTree& visited);
	bool ContainsAnotherPoints(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c, KDTree& visited);
	bool ConsistentNormal(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c);

	glm::vec3 ComputeCircumcenter(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c, float& bRadius);
	float ComputePivotingAngle(KDTreeNode* pA, KDTreeNode* pB, KDTreeNode* candidate);
	glm::vec3 RotateBall(glm::vec3& ballCenter, KDTreeNode* a, KDTreeNode* b, float angle);

public:
	BallPivoting(E57* e57);
	BallPivoting(E57* e57, float radius, float lowerRadius, float toleranceMultiplier);

	void SetRadius(float radius);
	void SetLowerRadius(float radius);

	void Run() override;
	void SetUp() override;
	~BallPivoting();
};

