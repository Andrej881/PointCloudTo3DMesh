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
		}
	}
	bool operator==(const Triangle2& other) const {
		return (p1 == other.p1 && p2 == other.p2 && p3 == other.p3) ||
			(p1 == other.p2 && p2 == other.p3 && p3 == other.p1) ||
			(p1 == other.p3 && p2 == other.p1 && p3 == other.p2) ||
			(p1 == other.p1 && p2 == other.p3 && p3 == other.p2) ||
			(p1 == other.p2 && p2 == other.p1 && p3 == other.p3) ||
			(p1 == other.p3 && p2 == other.p2 && p3 == other.p1);
	}
};

class BallPivoting : public ReconstructionAlgorithm
{
private:
	// Track processed points
	float radius, tolerance; 

	KDTree* tree;

	Triangle2 FindInitialTriangle();
	bool IsCenterOfTriangleValid(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c);
	bool IsTriangleValid(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c);
	bool ContainsAnotherPoints(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c);
	bool ConsistentNormal(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c);

	glm::vec3 ComputeCircumcenter(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c, float& bRadius);
	float ComputePivotingAngle(KDTreeNode* pA, KDTreeNode* pB, KDTreeNode* candidate);

public:
	BallPivoting(E57* e57);
	BallPivoting(E57* e57, float radius, float toleranceMultiplier);

	void SetRadius(float radius);

	void Run() override;
	void SetUp() override;

	~BallPivoting();
};

