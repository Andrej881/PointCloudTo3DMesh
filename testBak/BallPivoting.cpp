#include "BallPivoting.h"

bool BallPivoting::IsTriangleValid(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c)
{   
    if(!IsCenterOfTriangleValid(a, b, c))
		return false;   
    if (ContainsAnotherPoints(a, b, c))
        return false;

    return true;
}

glm::vec3 BallPivoting::ComputeBallCenter(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c, float& bRadius)
{
	glm::vec3 ballCenter = (a->point->position + b->point->position + c->point->position) / 3.0f;
	bRadius = glm::length(a->point->position - ballCenter);
    return ballCenter;
}

Triangle2 BallPivoting::FindInitialTriangle(std::unordered_set<KDTreeNode*>& visited)
{
    for (int i = 0; i < 15000; ++i)
    {
        printf("finding init triangle \n");
        KDTreeNode* seedPoint = nullptr;

        //int randomPointIndex = rand() % (e57->getCount() + 1);

        while (seedPoint == nullptr)
        {
            seedPoint = this->tree->GetRandomNode();
            //seedPoint = this->tree->FindNode(&e57->getPoints()[randomPointIndex]);
            if (visited.count(seedPoint))
                seedPoint = nullptr;
        }

        radius = initRadius;
        this->tolerance = this->radius * 0.05;
        int help = 0;
        for (int i = 0; i < 1000; ++i)
        {
            std::vector<KDTreeNode*> neighbors = tree->GetNeighborsWithinRadius(seedPoint, 2 * radius);
            bool inside = false;
            for (KDTreeNode* neighbor : neighbors)
            {
                if (neighbor == seedPoint || visited.count(neighbor))
                    continue;
                std::vector<KDTreeNode*> neighbors2 = tree->GetNeighborsWithinRadius(neighbor, 2 * radius);
                for (KDTreeNode* neighbor2 : neighbors2)
                {
                    if (neighbor == neighbor2 || seedPoint == neighbor2 || visited.count(neighbor2))
                        continue;
                    inside = ContainsAnotherPoints(seedPoint, neighbor, neighbor2);
                    if (!inside)
                    {
                        Triangle2 t = { seedPoint, neighbor, neighbor2 };
                        return t;
                    }
                }
            }
            this->radius += this->tolerance;
            if (inside)
            {
                help++;
                if(help > 3)
                    break;
            }
            if (this->radius > 1)
                break;
            this->tolerance = this->radius * 0.075;
            //printf("to small radius new radius %f\n", this->radius);
        }
    }
    return { nullptr, nullptr, nullptr };
    
}
bool BallPivoting::IsCenterOfTriangleValid(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c)
{
    float ballRadius;
    glm::vec3 ballCenter = ComputeBallCenter(a, b, c, ballRadius);

    if (ballRadius + this->tolerance < radius || ballRadius - this->tolerance > radius)
		return false;

    return true;
}

bool BallPivoting::ContainsAnotherPoints(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c)
{
    float ballRadius;
    glm::vec3 ballCenter = ComputeBallCenter(a, b, c, ballRadius);
    KDTreeNode ballCenterNode = { new E57Point{ballCenter}, nullptr, nullptr, nullptr };

    std::vector<KDTreeNode*> relevantPoints = tree->GetNeighborsWithinRadius(&ballCenterNode, ballRadius - this->tolerance);
    for (KDTreeNode* point : relevantPoints) {
        if (point != a && point != b && point != c)
            return true;
    }
    return false;
}

BallPivoting::BallPivoting(E57* e57) : ReconstructionAlgorithm(e57)
{
    this->tree = &e57->getTree();
	this->radius = this->initRadius = 0.005f;
    this->tolerance = this->radius * 0.075;
}

BallPivoting::BallPivoting(E57* e57, float radius, float tolerance) : ReconstructionAlgorithm(e57)
{
    this->tree = &e57->getTree();
	this->radius = this->initRadius = radius;
    this->tolerance = this->tolerance;
}

void BallPivoting::SetRadius(float radius)
{
	this->radius = radius;
}

void BallPivoting::Run()
{
    this->getTriangles().clear();
	std::vector<Triangle>& triangles = this->getTriangles();
    std::unordered_set<KDTreeNode*> visited;
    // Step 1: Find the initial seed triangle
    Triangle2 seedTriangle = FindInitialTriangle(visited);
    if (seedTriangle.p1 == nullptr || seedTriangle.p2 == nullptr || seedTriangle.p3 == nullptr)
        return;
    triangles.push_back(seedTriangle.triangle);
    visited.insert(seedTriangle.p1);
    visited.insert(seedTriangle.p2);
    visited.insert(seedTriangle.p3);

    // Step 2: Pivot and expand the mesh
    std::vector<Triangle2> frontier = { seedTriangle };
    while (!frontier.empty() || this->getTriangles().size() < 100000) 
    {
        if (frontier.empty())
        {
            seedTriangle = FindInitialTriangle(visited);
            if (seedTriangle.p1 == nullptr || seedTriangle.p2 == nullptr || seedTriangle.p3 == nullptr)
                break;
            triangles.push_back(seedTriangle.triangle);
            visited.insert(seedTriangle.p1);
            visited.insert(seedTriangle.p2);
            visited.insert(seedTriangle.p3);
            frontier.push_back(seedTriangle);
        }
        Triangle2 current = frontier.back();
        frontier.pop_back();

        // For each edge of the triangle, try to expand
        KDTreeNode* edges[3][2] = {
            {current.p1, current.p2},
            {current.p2, current.p3},
            {current.p3, current.p1}
        };

        float ballRadius;
        glm::vec3 ballCenter = ComputeBallCenter(current.p1, current.p1, current.p1, ballRadius);

        for (auto& edge : edges)
        {
            KDTreeNode* pA = edge[0];
            KDTreeNode* pB = edge[1];

            // Find potential new points to form a triangle
            std::vector<KDTreeNode*> candidates = tree->GetNeighborsOnRadius(ballCenter, radius, tolerance);            

            for (KDTreeNode*& candidate : candidates)
            {
                if (visited.count(candidate) || candidate == pA || candidate == pB) 
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
