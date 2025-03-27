#include "BallPivoting.h"

bool BallPivoting::IsTriangleValid(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c, KDTree& visited)
{   
    if(!IsCenterOfTriangleValid(a, b, c))
		return false;   
    if (ContainsAnotherPoints(a, b, c,visited))
        return false;

    return true;
}

glm::vec3 BallPivoting::ComputeBallCenter(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c, float& bRadius)
{
	glm::vec3 ballCenter = (a->point->position + b->point->position + c->point->position) / 3.0f;
	bRadius = glm::length(a->point->position - ballCenter);
    return ballCenter;
}

Triangle2 BallPivoting::FindInitialTriangle(std::unordered_set<KDTreeNode*>& visited, KDTree& visitedTree)
{
    for (int i = 0; i < 1000; ++i)
    {
        //printf("finding init triangle \n");
        KDTreeNode* seedPoint = nullptr;

        //int randomPointIndex = rand() % (e57->getCount() + 1);

        while (seedPoint == nullptr)
        {
            seedPoint = this->tree->GetRandomNode();
            //seedPoint = this->tree->FindNode(&e57->getPoints()[randomPointIndex]);
            if (visited.count(seedPoint))
                seedPoint = nullptr;
        }

        std::vector<KDTreeNode*> neighbors = tree->GetNeighborsWithinRadius(seedPoint, 2 * radius);
        bool inside = false;
        for (KDTreeNode* neighbor : neighbors)
        {
            if (neighbor == seedPoint || visited.count(neighbor))
                continue;
            for (KDTreeNode* neighbor2 : neighbors)
            {
                if (neighbor == neighbor2 || seedPoint == neighbor2 || visited.count(neighbor2))
                    continue;
                if (IsCenterOfTriangleValid(seedPoint, neighbor, neighbor2))
                {
                    inside = ContainsAnotherPoints(seedPoint, neighbor, neighbor2, visitedTree);
                    if (!inside)
                    {
                        Triangle2 t = { seedPoint, neighbor, neighbor2 };
                        return t;
                    }
                }
            }
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

bool BallPivoting::ContainsAnotherPoints(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c, KDTree& visited)
{
    float ballRadius;
    glm::vec3 ballCenter = ComputeBallCenter(a, b, c, ballRadius);
    KDTreeNode ballCenterNode = { new E57Point{ballCenter}, nullptr, nullptr, nullptr };

    std::vector<E57Point*> points;
    points.push_back(a->point);
    points.push_back(b->point);
    points.push_back(c->point);

    return visited.ContainsPointsWithinRadiusBesidesPoints(&ballCenterNode, points, ballRadius - this->tolerance);
}

BallPivoting::BallPivoting(E57* e57) : ReconstructionAlgorithm(e57)
{
    this->tree = &e57->getTree();
	this->radius = this->initRadius = 0.1f;
    this->lowerRadius = 0.01f;
    this->tolerance = this->radius * 0.05;
}

BallPivoting::BallPivoting(E57* e57, float radius,float lowerRadius, float tolerance) : ReconstructionAlgorithm(e57)
{
    this->tree = &e57->getTree();
	this->radius = this->initRadius = radius;
    this->tolerance = this->tolerance;
}
void BallPivoting::SetLowerRadius(float radius)
{
    this->lowerRadius = radius;
}
void BallPivoting::SetRadius(float radius)
{
	this->radius = radius;
}
int counter = 0;
void BallPivoting::Run()
{
    float radiusStep = (this->initRadius - this->lowerRadius) / 200;

    this->getTriangles().clear();
	std::vector<Triangle>& triangles = this->getTriangles();
    std::unordered_set<KDTreeNode*> visited;
    KDTree visitedTree;
    // Step 1: Find the initial seed triangle
    Triangle2 seedTriangle = FindInitialTriangle(visited, visitedTree);
    if (seedTriangle.p1 == nullptr || seedTriangle.p2 == nullptr || seedTriangle.p3 == nullptr)
        return;
    triangles.push_back(seedTriangle.triangle);
    visited.insert(seedTriangle.p1);
    visited.insert(seedTriangle.p2);
    visited.insert(seedTriangle.p3);
    visitedTree.Insert(seedTriangle.p1->point);
    visitedTree.Insert(seedTriangle.p2->point);
    visitedTree.Insert(seedTriangle.p3->point);

    // Step 2: Pivot and expand the mesh
    std::vector<Triangle2> frontier = { seedTriangle };
    while (!frontier.empty() || this->getTriangles().size() < 10000) 
    {
        if (frontier.empty())
        {
            radius -= radiusStep;
            tolerance = radius * 0.05;
            seedTriangle = FindInitialTriangle(visited, visitedTree);
            if (seedTriangle.p1 == nullptr || seedTriangle.p2 == nullptr || seedTriangle.p3 == nullptr)
            {
                printf("cur num of triangles %d\n", this->getTriangles().size());
                counter++;
                if (counter > 200)
                    break;
                continue;
            }
            radiusStep = (this->radius - this->lowerRadius) / 100;
            counter = 0;
            triangles.push_back(seedTriangle.triangle);
            visited.insert(seedTriangle.p1);
            visited.insert(seedTriangle.p2);
            visited.insert(seedTriangle.p3);
            visitedTree.Insert(seedTriangle.p1->point);
            visitedTree.Insert(seedTriangle.p2->point);
            visitedTree.Insert(seedTriangle.p3->point);
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
                if (IsTriangleValid(pA, pB, candidate,visitedTree))
                {

                    // Form a new triangle
                    Triangle2 newTriangle = { pA, pB, candidate};
                    triangles.push_back(newTriangle.triangle);
                    frontier.push_back(newTriangle);
                    visited.insert(candidate);
                    visitedTree.Insert(candidate->point);
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
