#include "BallPivoting.h"

bool BallPivoting::IsTriangleValid(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c, KDTree& visited)
{   
    float angle = glm::degrees(ComputePivotingAngle(a, b, c));
    if (angle <= 15 || angle >= 150)
        return false;

    if (!ConsistentNormal(a, b, c))
        return false;

    if (true || visited.GetRoot() != nullptr)
    {
        if (ContainsAnotherPoints(a, b, c, visited))
            return false;
    }

    return true;
}

glm::vec3 BallPivoting::ComputeCircumcenter(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c, float& bRadius)
{

    /*glm::vec3 AB = b->point->position - a->point->position;
    float d = glm::length(AB);
    if (d > 2 * radius) return glm::vec3(-5.0f); // Hrana je príliš dlhá
    glm::vec3 M = (a->point->position + b->point->position) * 0.5f;
    float h = sqrt(radius * radius - (d * 0.5f) * (d * 0.5f));
    // Vektor kolmý na AB v rovine definovanej bodmi A, B, C.
    glm::vec3 N = glm::normalize(glm::cross(AB, c->point->position - a->point->position));
    if (glm::length(N) < 1e-6) return glm::vec3(-5.0f); // Body sú príliš blízko kolineárne
    glm::vec3 perp = glm::normalize(glm::cross(N, AB));
    // Dve možné kandidátske polohy gu¾ového stredu.
    glm::vec3 center1 = M + h * perp;
    glm::vec3 center2 = M - h * perp;
    // Vyberieme tú možnos, ktorej vzdialenos od bodu C je bližšie k r.
    float err1 = fabs(glm::length(center1 - c->point->position) - radius);
    float err2 = fabs(glm::length(center2 - c->point->position) - radius);
    if (err1 < err2)
        return center1;
    else
        return center2;*/

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

bool fristSearch = true;
Triangle2 BallPivoting::FindInitialTriangle(std::unordered_set<KDTreeNode*>& visited, KDTree& visitedTree)
{
    int iterations = fristSearch ? 1000 : 1;
    for (int i = 0; i < iterations; ++i)
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
                return { nullptr, nullptr, nullptr};
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
                    if (IsTriangleValid(seedPoint, neighbor, neighbor2, visitedTree))
                    {
                        Triangle2 t = { seedPoint, neighbor, neighbor2 };
                        return t;
                    }
                }
            }
        }
    }
    return { nullptr, nullptr, nullptr};
    
}
bool BallPivoting::IsCenterOfTriangleValid(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c)
{
    float ballRadius;
    glm::vec3 ballCenter = ComputeCircumcenter(a, b, c, ballRadius);

    if (ballRadius + this->tolerance < radius || ballRadius - this->tolerance > radius)
		return false;

    return true;
}

bool BallPivoting::ContainsAnotherPoints(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c, KDTree& visited)
{
    float rad;
	glm::vec3 center = ComputeCircumcenter(a, b, c, rad);
	/*if (center.x == -5.0f)
		return false;*/
    E57Point pC = { center };
    KDTreeNode ballCenterNode = { &pC, nullptr, nullptr, nullptr };

    std::vector<E57Point*> points;
    points.push_back(a->point);
    points.push_back(b->point);
    points.push_back(c->point);

    bool contains = tree->ContainsPointsWithinRadiusBesidesPoints(&ballCenterNode, points, rad -this->tolerance);
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
    return isAConsistent == isBConsistent && isBConsistent == isCConsistent;

    
}

BallPivoting::BallPivoting(E57* e57) : ReconstructionAlgorithm(e57)
{
    this->tree = &e57->getTree();
	this->radius = this->initRadius = 0.02f;
    this->lowerRadius = 0.01f;
    this->tolerance = this->radius * 0.05;
}

BallPivoting::BallPivoting(E57* e57, float radius,float lowerRadius, float toleranceMultiplier) : ReconstructionAlgorithm(e57)
{
    this->tree = &e57->getTree();
	this->radius = this->initRadius = radius;
    this->tolerance = radius * toleranceMultiplier;
}
void BallPivoting::SetLowerRadius(float radius)
{
    this->lowerRadius = radius;
}
void BallPivoting::SetRadius(float radius)
{
	this->radius = radius;
}

struct EdgeHash
{
	std::size_t operator()(const Edge& e) const {
        std::size_t h1 = std::hash<KDTreeNode*>()(e.a);
        std::size_t h2 = std::hash<KDTreeNode*>()(e.b);
        return h1 ^ h2;
    }
};
int counter = 0;
void BallPivoting::Run()
{
	int numOfRadiusSteps = 2;
    float radiusStep = numOfRadiusSteps > 0 ? (this->initRadius - this->lowerRadius) / numOfRadiusSteps : 0;

    this->GetTriangles().clear();
	std::vector<Triangle>& triangles = this->GetTriangles();
    std::unordered_set<KDTreeNode*> visited;
    std::unordered_set<Edge, EdgeHash> visited2;

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

    visited2.insert({ seedTriangle.p1 ,seedTriangle.p2 });
    visited2.insert({ seedTriangle.p1 ,seedTriangle.p3 });
    visited2.insert({ seedTriangle.p2 ,seedTriangle.p3 });    

    // Step 2: Pivot and expand the mesh
    //std::vector<Triangle2> frontier = { seedTriangle };
    
    std::vector<Edge> frontier2 = { {seedTriangle.p1, seedTriangle.p2} };
    frontier2.push_back({ seedTriangle.p1, seedTriangle.p3 });
    frontier2.push_back({ seedTriangle.p2, seedTriangle.p3 });
    while (!frontier2.empty()) 
    {
        if (this->GetTriangles().size() % 1000 == 0)
            //printf("[mod1000] cur num of triangles %d num of trinagles in front %d\n", this->GetTriangles().size(), frontier.size());
            printf("[mod1000] cur num of triangles %d num of edges in front %d\n", this->GetTriangles().size(), frontier2.size());
        if (frontier2.empty())
        {
            radius -= radiusStep;
            tolerance = radius * 0.05;
            printf("[NEW INIT] cur num of triangles %d\n", this->GetTriangles().size());
            seedTriangle = FindInitialTriangle(visited, visitedTree);
            printf("[FOUND]\n");
            if (seedTriangle.p1 == nullptr || seedTriangle.p2 == nullptr || seedTriangle.p3 == nullptr)
            {
                printf("[NEW INIT NULL] cur num of triangles %d\n", this->GetTriangles().size());
                counter++;
                if (counter > numOfRadiusSteps)
                    break;
                continue;
            }
            radius = initRadius;
            tolerance = radius * 0.05;
            counter = 0;
            triangles.push_back(seedTriangle.triangle);
            visited.insert(seedTriangle.p1);
            visited.insert(seedTriangle.p2);
            visited.insert(seedTriangle.p3);
            visitedTree.Insert(seedTriangle.p1->point);
            visitedTree.Insert(seedTriangle.p2->point);
            visitedTree.Insert(seedTriangle.p3->point);
			visited2.insert({ seedTriangle.p1 ,seedTriangle.p2 });
			visited2.insert({ seedTriangle.p1 ,seedTriangle.p3 });
			visited2.insert({ seedTriangle.p2 ,seedTriangle.p3 });
            //frontier.push_back(seedTriangle);
            frontier2.push_back({ seedTriangle.p1, seedTriangle.p2 });
            frontier2.push_back({ seedTriangle.p1, seedTriangle.p3 });
            frontier2.push_back({ seedTriangle.p2, seedTriangle.p3 });
        }

        Edge current = frontier2.back();
        frontier2.pop_back();

        KDTreeNode* pA = current.a;
        KDTreeNode* pB = current.b;

        glm::vec3 midpoint = (pA->point->position + pB->point->position) / 2.0f;
        E57Point m = { midpoint ,glm::vec3(0.0f), false };
        KDTreeNode node = { &m,nullptr,nullptr,nullptr };

        bool stop = false;
        for (int i = 0; i < 1; ++i)
        {   
            if(stop)
				break;

            std::vector<KDTreeNode*> candidates = tree->GetNeighborsWithinRadius(&node, radius * 2);

            std::sort(candidates.begin(), candidates.end(), [&](KDTreeNode* a, KDTreeNode* b)
                {
                    /*float angleA = glm::degrees(ComputePivotingAngle(pA, pB, a));
                    float angleB = glm::degrees(ComputePivotingAngle(pA, pB, b));

                    return angleA < angleB;*/

					/*float distanceA = glm::length(a->point->position - midpoint);
					float distanceB = glm::length(b->point->position - midpoint);
					return distanceA < distanceB;*/

                    glm::vec3 ballCenterA = (midpoint + a->point->position) / 2.0f;
					glm::vec3 ballCenterB = (midpoint + b->point->position) / 2.0f;

					float distA = glm::length(ballCenterA - a->point->position);
					float distB = glm::length(ballCenterB - b->point->position);

					return distA < distB;
                });

            for (KDTreeNode*& candidate : candidates)
            {

                glm::vec3 ballCenter = (midpoint + candidate->point->position) / 2.0f;
                float dist = glm::length(ballCenter - candidate->point->position);
                if (dist > radius + tolerance)
                {
                    stop = true;
                    break;
                }

                if (candidate == pA || candidate == pB)
                    continue;  // Skip already processed points;            

				if (visited2.count({ pA, candidate }) && visited2.count({ pB, candidate }))
					continue;  

                // Check if the new triangle is valid
                if (IsTriangleValid(pA, pB, candidate, visitedTree))
                {
                    // Form a new triangle
                    Triangle2 newTriangle = { pA, pB,candidate };
                    triangles.push_back(newTriangle.triangle);
                    //frontier2.push_back({ newTriangle.p1, newTriangle.p2 });

					// Add the new edges to the frontier

                    if (!visited2.count({ pA, pB }))
                        frontier2.push_back({ pA, pB });

					if (!visited2.count({ pA, candidate }))
						frontier2.push_back({ pA, candidate });

					if (!visited2.count({ pB, candidate }))
						frontier2.push_back({ pB, candidate });

                    visited.insert(candidate);
                    visitedTree.Insert(candidate->point);
                    visited2.insert({ pA, pB });
                    visited2.insert({ pA, candidate });
                    visited2.insert({ pB, candidate });
                    stop = true;
                    break;  // Only add one new triangle per edge
                }
            }
            radius -= radiusStep;
            tolerance = radius * 0.05;
        }
		this->radius = this->initRadius;
        tolerance = radius * 0.05;
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
