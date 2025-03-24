#include "BallPivoting.h"

bool BallPivoting::IsTriangleValid(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c)
{   
    /*
    // 1. Overenie, Ëi body nie s˙ koline·rne
    glm::vec3 v1 = b->point->position - a->point->position;
    glm::vec3 v2 = c->point->position - a->point->position;
    glm::vec3 normal = glm::cross(v1, v2);

    if (glm::length(normal) < 1e-6) {
        return false;
    }*/

    // 2. N·jsù stred a polomer gule prech·dzaj˙cej bodmi (circumsphere)
    glm::vec3 ballCenter = (a->point->position + b->point->position + c->point->position) / 3.0f;	
	float ballRadius = glm::length(a->point->position - ballCenter);

    // 3. Overiù, Ëi neexistuj˙ inÈ body vn˙tri tejto gule

    std::vector<KDTreeNode*> relevantPoints = tree->GetNeighborsWithinRadius(a, ballRadius);
    for (KDTreeNode* point : relevantPoints) {
        if (point != a && point != b && point != c) {
			float dist = glm::length(point->point->position - ballCenter);
			if (dist < ballRadius) {
				return false;
			}            
        }
    }
    /*
    // 4. Overenie norm·lov˝ch vektorov (uhol medzi norm·lami nesmie byù prÌliö veæk˝)
    glm::vec3 normalA = a->point->normal;
    glm::vec3 normalB = b->point->normal;
    glm::vec3 normalC = c->point->normal;

    float angleA = glm::dot(normalA, normal);
    float angleB = glm::dot(normalB, normal);
    float angleC = glm::dot(normalC, normal);

    if (angleA < 0.5f || angleB < 0.5f || angleC < 0.5f) {
        return false;
    }*/

    return true;
}

Triangle2 BallPivoting::FindInitialTriangle()
{
    KDTreeNode* seedPoint = this->tree->GetRoot();
    if (!seedPoint) {
        throw std::runtime_error("Point cloud is empty!");
    }

    // Get neighbors within the ball radius
    std::vector<KDTreeNode*> neighbors = tree->GetNeighborsOnRadius(seedPoint, radius);

    // Find two neighbors that form a valid triangle
    for (size_t i = 0; i < neighbors.size(); ++i)
    {
        for (size_t j = i + 1; j < neighbors.size(); ++j) 
        {
            // Ensure points are not collinear
            if (IsTriangleValid(seedPoint, neighbors[i], neighbors[j]))
            {
                Triangle2 t = { seedPoint, neighbors[i], neighbors[j] };
                return t;
            }
        }
    }

    return {nullptr , nullptr, nullptr};
}

BallPivoting::BallPivoting(E57* e57) : ReconstructionAlgorithm(e57)
{
    this->tree = &e57->getTree();
	this->radius = 0.05f;
}

BallPivoting::BallPivoting(E57* e57, float radius) : ReconstructionAlgorithm(e57)
{
    this->tree = &e57->getTree();
	this->radius = radius;
}

void BallPivoting::SetRadius(float radius)
{
	this->radius = radius;
}

void BallPivoting::Run()
{
    this->getTriangles().clear();
	std::vector<Triangle>& triangles = this->getTriangles();
    std::unordered_set<KDTreeNode*> visited;  // Track processed points

    // Step 1: Find the initial seed triangle
    Triangle2 seedTriangle = FindInitialTriangle();
    if (seedTriangle.p1 == nullptr || seedTriangle.p2 == nullptr || seedTriangle.p3 == nullptr)
        return;
    triangles.push_back(seedTriangle.triangle);

    visited.insert(seedTriangle.p1);
    visited.insert(seedTriangle.p2);
    visited.insert(seedTriangle.p3);

    // Step 2: Pivot and expand the mesh
    std::vector<Triangle2> frontier = { seedTriangle };

    while (!frontier.empty()) 
    {
        Triangle2 current = frontier.back();
        frontier.pop_back();

        // For each edge of the triangle, try to expand
        KDTreeNode* edges[3][2] = {
            {current.p1, current.p2},
            {current.p2, current.p3},
            {current.p3, current.p1}
        };

        for (auto& edge : edges)
        {
            KDTreeNode* pA = edge[0];
            KDTreeNode* pB = edge[1];

            // Find potential new points to form a triangle
            std::vector<KDTreeNode*> candidates = tree->GetNeighborsOnRadius(pA, radius);

            for (KDTreeNode* candidate : candidates)
            {
                if (visited.count(candidate)) 
                    continue;  // Skip already processed points
                                
                // Check if the new triangle is valid
                if (IsTriangleValid(pA, pB, candidate))
                {

                    // Form a new triangle
                    Triangle2 newTriangle = { pA, pB, candidate};
                    triangles.push_back(newTriangle.triangle);
                    frontier.push_back(newTriangle);
                    visited.insert(candidate);
                    break;  // Only add one new triangle per edge
                }
            }
        }
    }
}

void BallPivoting::SetUp()
{
    //if(!e57->GetHasNormals())
        //e57->CalculateNormals();
    if (tree->GetRoot() == nullptr)
        e57->SetUpTree();
}

BallPivoting::~BallPivoting()
{
}
