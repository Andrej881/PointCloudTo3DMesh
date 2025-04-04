#include "BallPivoting.h"

bool BallPivoting::IsTriangleValid(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c, KDTree& visited, glm::vec3 ballCenter)
{   
    if (glm::length(a->point->position - b->point->position) > 2 * radius || glm::length(c->point->position - b->point->position) > 2 * radius || glm::length(a->point->position - c->point->position) > 2 * radius)
    {
        //printf("AB[%f], BC[%f], AC[%f]", glm::length(a->point->position - b->point->position), glm::length(c->point->position - b->point->position), glm::length(a->point->position - c->point->position));
        return false;
    }
    if (a->point->position == b->point->position || b->point->position == c->point->position || c->point->position == a->point->position) 
        return false;

    if (!ConsistentNormal(a, b, c))
        return false;

    if (visited.GetRoot() != nullptr)
    {
        if (ContainsAnotherPoints(a, b, c, visited, ballCenter))
            return false;
    }

    return true;
}

glm::vec3 BallPivoting::ComputeCircumcenter(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c, float& bRadius)
{
    glm::vec3 A = a->point->position;
    glm::vec3 B = b->point->position;
    glm::vec3 C = c->point->position;

    glm::vec3 AB = B - A;
    glm::vec3 AC = C - A;
    glm::vec3 BC = C - B;

    // Compute midpoints of two edges
    glm::vec3 midAB = (A + B) * 0.5f;
    glm::vec3 midAC = (A + C) * 0.5f;

    // Normal of the triangle
    glm::vec3 normal = glm::cross(AB, AC);

    // Solve for circumcenter using linear algebra
    glm::vec3 bisectorAB = glm::cross(normal, AB);
    glm::vec3 bisectorAC = glm::cross(normal, AC);

    glm::mat3 M = glm::mat3(bisectorAB, -bisectorAC, normal); // Matrix for the system
    glm::vec3 d = midAC - midAB;

    glm::vec3 solution = glm::inverse(M) * d;
    glm::vec3 ballPosition = midAB + solution.x * bisectorAB;

    // Compute correct ball radius
    bRadius = glm::length(ballPosition - A);

    //printf("BC [%f, %f, %f]\n", ballPosition.x, ballPosition.y, ballPosition.z);
    //printf("radius [%f, %f, %f]\n",bRadius , glm::length(ballPosition - B), glm::length(ballPosition - C));

    return ballPosition;
}

Triangle2 BallPivoting::FindInitialTriangle(std::unordered_set<KDTreeNode*>& visited, KDTree& visitedTree)
{
    for (int i = 0; i < 1; ++i)
    {
        //printf("finding init triangle \n");
        KDTreeNode* seedPoint = nullptr;

        //int randomPointIndex = rand() % (e57->getCount() + 1);
        int help = 0;
        while (seedPoint == nullptr)
        {
            seedPoint = this->tree->GetRandomNode();
            //seedPoint = this->tree->FindNode(&e57->getPoints()[randomPointIndex]);
            if (visited.count(seedPoint))
                seedPoint = nullptr;
            help++;
            if (help > this->e57->getCount() / 4)
                break;
        }

        std::vector<KDTreeNode*> neighbors = tree->GetNeighborsWithinRadius(seedPoint, 2 * radius);
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
                    float ballRadius;
                    glm::vec3 ballCenter = ComputeCircumcenter(seedPoint, neighbor, neighbor2, ballRadius);
                    if (IsTriangleValid(seedPoint, neighbor, neighbor2, visitedTree, ballCenter))
                    {
                        Triangle2 t = { seedPoint, neighbor, neighbor2, ballCenter };
                        return t;
                    }
                }
            }
        }
    }
    return { nullptr, nullptr, nullptr, glm::vec3(0)};
    
}
bool BallPivoting::IsCenterOfTriangleValid(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c)
{
    float ballRadius;
    glm::vec3 ballCenter = ComputeCircumcenter(a, b, c, ballRadius);

    if (ballRadius + this->tolerance < radius || ballRadius - this->tolerance > radius)
		return false;

    return true;
}

bool BallPivoting::ContainsAnotherPoints(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c, KDTree& visited, glm::vec3 ballCenter)
{
    KDTreeNode ballCenterNode = { new E57Point{ballCenter}, nullptr, nullptr, nullptr };

    std::vector<E57Point*> points;
    points.push_back(a->point);
    points.push_back(b->point);
    points.push_back(c->point);

    bool contains = tree->ContainsPointsWithinRadiusBesidesPoints(&ballCenterNode, points, radius);// -this->tolerance);
    delete ballCenterNode.point;
    return contains;
}

bool BallPivoting::ConsistentNormal(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c)
{
    // Calculate the triangle normal using the cross product of two edges
    glm::vec3 edge1 = b->point->position - a->point->position;
    glm::vec3 edge2 = c->point->position - a->point->position;
    glm::vec3 triangleNormal = glm::normalize(glm::cross(edge1, edge2));

    // Dot product checks to see if the vertex normals are consistent with the triangle normal
    bool isAConsistent = glm::dot(triangleNormal, a->point->normal) > 0;
    bool isBConsistent = glm::dot(triangleNormal, b->point->normal) > 0;
    bool isCConsistent = glm::dot(triangleNormal, c->point->normal) > 0;

    // If all vertex normals are consistent, return true
    return isAConsistent && isBConsistent && isCConsistent;
}

std::vector<glm::vec3> BallPivoting::GetPossibleCenters(glm::vec3 A, glm::vec3 B, glm::vec3 midpoint, glm::vec3 C)
{
    std::vector<glm::vec3> possibleCenters = std::vector<glm::vec3>();

    glm::vec3 AB = B - A;
    glm::vec3 dir = glm::normalize(AB);

    // Najdenie najblizsieho bodu na usecke AB k bodu P
    glm::vec3 closest = A + glm::proj(C - A, AB);
    float distToLineSq = glm::distance2(C, closest);

    if (distToLineSq > radius * radius) {
        return possibleCenters; // Ziadne riesenie
    }

    // Vzdialenost medzi najblizsim bodom a hladanym stredom
    float h = sqrt(radius * radius - distToLineSq);

    // Vektor ortogonalny na AB cez P
    glm::vec3 ortho = glm::normalize(C - closest);
    if (glm::length(ortho) == 0) {
        // Ak je P presne na usecke, hladane stredy su na kruznici
        ortho = glm::vec3(1, 0, 0); // Zvol nejaku lubovolnu os
    }

    // Mozne stredy S1 a S2
    glm::vec3 center1 = closest + h * ortho;
    glm::vec3 center2 = closest - h * ortho;

    possibleCenters.push_back(center1);
    possibleCenters.push_back(center2);

    return possibleCenters;
}

BallPivoting::BallPivoting(E57* e57) : ReconstructionAlgorithm(e57)
{
    this->tree = &e57->getTree();
	this->radius = this->initRadius = 0.05f;
    this->lowerRadius = 0.04f;
    this->tolerance = this->radius * 0.05;
    this->rotationAngle = 5;
}

BallPivoting::BallPivoting(E57* e57, float radius,float lowerRadius, float toleranceMultiplier, float rotationAngle) : ReconstructionAlgorithm(e57)
{
    this->tree = &e57->getTree();
	this->radius = this->initRadius = radius;
    this->tolerance = radius * toleranceMultiplier;
    this->rotationAngle = rotationAngle;
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
    float radiusStep = (this->initRadius - this->lowerRadius) / 20;

    this->GetTriangles().clear();
	std::vector<Triangle>& triangles = this->GetTriangles();
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
    while (!frontier.empty() || this->GetTriangles().size() < 100000) 
    {
        if (this->GetTriangles().size() % 1000 == 0)
            printf("[mod1000] cur num of triangles %d num of trinagles in front %d\n", this->GetTriangles().size(), frontier.size());
        if (frontier.empty())
        {
            break;
            radius -= radiusStep;
            tolerance = radius * 0.05;
            printf("[NEW INIT] cur num of triangles %d\n", this->GetTriangles().size());
            seedTriangle = FindInitialTriangle(visited, visitedTree);
            printf("[FOUND]\n");
            if (seedTriangle.p1 == nullptr || seedTriangle.p2 == nullptr || seedTriangle.p3 == nullptr)
            {
                printf("[NEW INIT NULL] cur num of triangles %d\n", this->GetTriangles().size());
                counter++;
                if (counter > 20)
                    break;
                continue;
            }
            radiusStep = (this->radius - this->lowerRadius) / 20;
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
        for (auto& edge : edges)
        {
            KDTreeNode* pA = edge[0];
            KDTreeNode* pB = edge[1];
            glm::vec3 ballCenter = current.ballCenter;

            glm::vec3 midPoint = (pA->point->position + pB->point->position) / 2.0f;
            E57Point m = { midPoint ,glm::vec3(0.0f), false };
            KDTreeNode node = {&m,nullptr,nullptr,nullptr};

            float r = glm::length(midPoint - ballCenter);

            //std::vector<KDTreeNode*> candidates = tree->GetNeighborsWithinRadius(&node,r*2);
            std::vector<KDTreeNode*> candidates = tree->GetNeighborsWithinToroidalRadius(pA->point->position, pB->point->position, radius);
            bool createdTriangle = false;
            for (KDTreeNode*& candidate : candidates)
            {
                if (createdTriangle)
                    break;
                if (visited.count(candidate) || candidate == pA || candidate == pB)
                    continue;  // Skip already processed points
                std::vector<glm::vec3> centers = GetPossibleCenters(pA->point->position, pB->point->position, midPoint, candidate->point->position);

                for (glm::vec3& center : centers)
                {
                    // Check if the new triangle is valid
                    if (IsTriangleValid(pA, pB, candidate, visitedTree, center))
                    {
                        // Form a new triangle
                        Triangle2 newTriangle = { pA, pB,candidate, center };
                        triangles.push_back(newTriangle.triangle);
                        frontier.push_back(newTriangle);
                        visited.insert(candidate);
                        visitedTree.Insert(candidate->point);
                        createdTriangle = true;
                        break;  // Only add one new triangle per edge
                    }
                }                
            }

            /*glm::vec3 rotationAxis = glm::normalize(pB->point->position - pA->point->position);
            bool createdTriangle = false;
            for (float i = 90; i <= 270; i += this->rotationAngle)
            {
                if (createdTriangle)
                    break;

                ballCenter = RotateBall(current.ballCenter, pA, pB, i);
                std::vector<KDTreeNode*> candidates = tree->GetNeighborsOnRadius(ballCenter, radius, tolerance);

                for (KDTreeNode*& candidate : candidates)
                {
                    if (visited.count(candidate) || candidate == pA || candidate == pB)
                        continue;  // Skip already processed points

                    // Check if the new triangle is valid
                    if (IsTriangleValid(candidate, pA, pB, visitedTree, ballCenter))
                    {
                        // Form a new triangle
                        Triangle2 newTriangle = { candidate, pA, pB, ballCenter };
                        triangles.push_back(newTriangle.triangle);
                        frontier.push_back(newTriangle);
                        visited.insert(candidate);
                        visitedTree.Insert(candidate->point);
                        createdTriangle = true;
                        break;  // Only add one new triangle per edge
                    }
                }
            }  */

        }
    }
}

void BallPivoting::SetUp()
{
    if(!e57->GetHasNormals())
        e57->CalculateNormals();
    if (tree->GetRoot() == nullptr)
        e57->SetUpTree();
}

float BallPivoting::ComputePivotingAngle(KDTreeNode* pA, KDTreeNode* pB, KDTreeNode* candidate)
{
    glm::vec3 edge = glm::normalize(pB->point->position - pA->point->position);
    glm::vec3 toCandidate = glm::normalize(candidate->point->position - (pA->point->position + pB->point->position) * 0.5f);

    return acos(glm::dot(edge, toCandidate));  // Angle in radians
}

glm::vec3 BallPivoting::RotateBall(glm::vec3& ballCenter, KDTreeNode* a, KDTreeNode* b, float angle)
{
    float angleInRadians = glm::radians(angle);

    // Compute the rotation axis (normalized edge vector)
    glm::vec3 axis = glm::normalize(b->point->position - a->point->position);

    // Compute the quaternion rotation
    glm::quat rotation = glm::angleAxis(angleInRadians, axis);

    // Translate point to origin (relative to edgeStart)
    glm::vec3 translatedPoint = ballCenter - a->point->position;

    // Rotate the point using quaternion
    glm::vec3 rotatedPoint = rotation * translatedPoint;

    // Translate back to original position
    return rotatedPoint + a->point->position;
}

BallPivoting::~BallPivoting()
{
}
