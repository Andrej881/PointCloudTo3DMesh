#include "BallPivoting.h"

bool BallPivoting::IsTriangleValid(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c)
{   
    float angle = glm::degrees(ComputePivotingAngle(a, b, c));
    if (angle <= 10 || angle >= 170)
        return false;

    if (e57->GetHasNormals() && !ConsistentNormal(a, b, c))
       return false;

    if (ContainsAnotherPoints(a, b, c))
        return false;

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

Triangle2 BallPivoting::FindInitialTriangle()
{
    int iterations = 1000;
    for (int i = 0; i < iterations; ++i)
    {
        KDTreeNode* seedPoint = this->tree->GetRandomNode();

        std::vector<KDTreeNode*> neighbors = tree->GetNeighborsWithinRadius(seedPoint->point->position, 2 * radius);
        for (KDTreeNode* neighbor : neighbors)
        {
            if (neighbor == seedPoint)
                continue;
            for (KDTreeNode* neighbor2 : neighbors)
            {
                if (this->stopEarly)
                    return { nullptr, nullptr, nullptr };
                if (neighbor == neighbor2 || seedPoint == neighbor2)
                    continue;
                if (IsCenterOfTriangleValid(seedPoint, neighbor, neighbor2))
                {
                    if (IsTriangleValid(seedPoint, neighbor, neighbor2))
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
Triangle2 BallPivoting::FindNextInitialTriangle(std::unordered_set<Triangle2, Triangle2Hash>& visitedTriangles)
{
    int iterations = 1000;
    for (int i = 0; i < iterations; ++i)
    {
        if(i % (iterations / 10) == 0 && i != 0)
            printf("Finding Next Triangle %d%% until it ends\n", (i * 100) / iterations);
        KDTreeNode* seedPoint = this->tree->GetRandomNode();

        std::vector<KDTreeNode*> neighbors = tree->GetNeighborsWithinRadius(seedPoint->point->position, 2 * radius);
        for (KDTreeNode* neighbor : neighbors)
        {
            if (neighbor == seedPoint)
                continue;
            for (KDTreeNode* neighbor2 : neighbors)
            {
                if (this->stopEarly)
                    return { nullptr, nullptr, nullptr };
                if (neighbor == neighbor2 || seedPoint == neighbor2 || visitedTriangles.count({seedPoint, neighbor, neighbor2}))
                    continue;
                if (IsCenterOfTriangleValid(seedPoint, neighbor, neighbor2))
                {
                    if (IsTriangleValid(seedPoint, neighbor, neighbor2))
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
    glm::vec3 ballCenter = ComputeCircumcenter(a, b, c, ballRadius);

    if (ballRadius - this->tolerance > radius)
		return false;

    return true;
}

bool BallPivoting::ContainsAnotherPoints(KDTreeNode* a, KDTreeNode* b, KDTreeNode* c)
{
    float rad;
	glm::vec3 center = ComputeCircumcenter(a, b, c, rad);

    std::unordered_set<E57Point*> points;
    points.insert(a->point);
    points.insert(b->point);
    points.insert(c->point);

    bool contains = tree->ContainsPointsWithinRadiusBesidesPoints(center, points, rad -this->tolerance);
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
	this->radius = 0.015f;
    this->tolerance = this->radius * 0.05;
}

BallPivoting::BallPivoting(E57* e57, float radius, float toleranceMultiplier) : ReconstructionAlgorithm(e57)
{
    this->tree = &e57->getTree();
	this->radius = radius;
    this->tolerance = radius * toleranceMultiplier;
}
void BallPivoting::SetRadius(float radius)
{
	this->radius = radius;
}

int counter = 0;
void BallPivoting::Run()
{
	auto start = std::chrono::high_resolution_clock::now();

    this->running = true;

    std::unique_lock<std::mutex> lock(triangleMutex);
    this->GetTriangles().clear();
    lock.unlock();

	std::vector<Triangle>& triangles = this->GetTriangles();
    std::unordered_set<Edge, EdgeHash> visited2;
    std::unordered_set<Triangle2, Triangle2Hash> visited3;

    // Step 1: Find the initial seed triangle
    Triangle2 seedTriangle = FindInitialTriangle();
    if (seedTriangle.p1 == nullptr || seedTriangle.p2 == nullptr || seedTriangle.p3 == nullptr)
    {
        this->stopEarly = this->running = false;
        return;
    }

    lock = std::unique_lock<std::mutex>(triangleMutex);
    triangles.push_back(seedTriangle.triangle);
    lock.unlock();

    visited2.insert({ seedTriangle.p1 ,seedTriangle.p2 });
    visited2.insert({ seedTriangle.p1 ,seedTriangle.p3 });
    visited2.insert({ seedTriangle.p2 ,seedTriangle.p3 });    

	visited3.insert(seedTriangle);

    // Step 2: Pivot and expand the mesh
    
    std::vector<Edge> frontier2 = { {seedTriangle.p1, seedTriangle.p2} };
    frontier2.push_back({ seedTriangle.p1, seedTriangle.p3 });
    frontier2.push_back({ seedTriangle.p2, seedTriangle.p3 });
    while (!this->stopEarly) 
    {
        //if (frontier2.empty())
            //break;
        bool end = false;
        while (frontier2.empty())
        {			
			seedTriangle = FindNextInitialTriangle(visited3);
            if (seedTriangle.p1 == nullptr || seedTriangle.p2 == nullptr || seedTriangle.p3 == nullptr)
            {
                end = true;
				break;
            }

			lock = std::unique_lock<std::mutex>(triangleMutex);
			triangles.push_back(seedTriangle.triangle);
			lock.unlock();
            if (!visited2.count({ seedTriangle.p1 ,seedTriangle.p2 }))
                frontier2.push_back({ seedTriangle.p1 ,seedTriangle.p2 });

            if (!visited2.count({ seedTriangle.p1 ,seedTriangle.p3 }))
                frontier2.push_back({ seedTriangle.p1 ,seedTriangle.p3 });

            if (!visited2.count({ seedTriangle.p2 ,seedTriangle.p3 }))
                frontier2.push_back({ seedTriangle.p2 ,seedTriangle.p3 });

			visited2.insert({ seedTriangle.p1 ,seedTriangle.p2 });
			visited2.insert({ seedTriangle.p1 ,seedTriangle.p3 });
			visited2.insert({ seedTriangle.p2 ,seedTriangle.p3 });
			visited3.insert(seedTriangle);
        }

        if (end)
            break;

        if (this->GetTriangles().size() % 1000 == 0)
            //printf("[mod1000] cur num of triangles %d num of trinagles in front %d\n", this->GetTriangles().size(), frontier.size());
            printf("[mod1000] cur num of triangles %d num of edges in front %d\n", this->GetTriangles().size(), frontier2.size());
        
        Edge current = frontier2.back();
        frontier2.pop_back();

        KDTreeNode* pA = current.a;
        KDTreeNode* pB = current.b;

        glm::vec3 midpoint = (pA->point->position + pB->point->position) / 2.0f;

        std::vector<KDTreeNode*> candidates = tree->GetNeighborsWithinRadius(midpoint, radius * 2);

        std::sort(candidates.begin(), candidates.end(), [&](KDTreeNode* a, KDTreeNode* b)
            {
                glm::vec3 ballCenterA = (midpoint + a->point->position) / 2.0f;
                glm::vec3 ballCenterB = (midpoint + b->point->position) / 2.0f;

                float distA2 = glm::distance2(ballCenterA, a->point->position);
                float distB2 = glm::distance2(ballCenterB, b->point->position);

                return distA2 < distB2;
            });

        for (KDTreeNode*& candidate : candidates)
        {

            glm::vec3 ballCenter = (midpoint + candidate->point->position) / 2.0f;
            float dist = glm::length(ballCenter - candidate->point->position);
            if (dist > radius + tolerance)
            {
                break;
            }

            if (candidate == pA || candidate == pB)
                continue;  // Skip already processed points;            

            /*if (visited2.count({pA, candidate}) && visited2.count({pB, candidate}))
                continue;*/

            if (visited3.count({ pA, pB, candidate }))
                continue;

            // Check if the new triangle is valid
            if (IsTriangleValid(pA, pB, candidate))
            {
                // Form a new triangle
                Triangle2 newTriangle = { pA, pB,candidate };

                lock = std::unique_lock<std::mutex>(triangleMutex);
                triangles.push_back(newTriangle.triangle);
                lock.unlock();

                // Add the new edges to the frontier
                if (!visited2.count({ pA, pB }))
                    frontier2.push_back({ pA, pB });

                if (!visited2.count({ pA, candidate }))
                    frontier2.push_back({ pA, candidate });

                if (!visited2.count({ pB, candidate }))
                    frontier2.push_back({ pB, candidate });

                visited2.insert({ pA, pB });
                visited2.insert({ pA, candidate });
                visited2.insert({ pB, candidate });

                visited3.insert(newTriangle);

                break;  // Only add one new triangle per edge
            }
        }
    }
    this->running = this->stopEarly = false;

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::unique_lock<std::mutex> lock2(triangleMutex);
    std::cout << "Trvanie algoritmu: " << duration.count() << " sec. a trojuholnikov " << this->GetTriangles().size() << std::endl;
    lock2.unlock();
}

void BallPivoting::SetUp()
{
    /*if(!e57->GetHasNormals())
        e57->CalculateNormals(radius,0);*/
    if (tree->GetRoot() == nullptr)
        e57->SetUpTree();
}
float BallPivoting::ComputePivotingAngle(KDTreeNode* pA, KDTreeNode* pB, KDTreeNode* candidate)
{
    glm::vec3 edge = glm::normalize(pB->point->position - pA->point->position);
    glm::vec3 toCandidate = glm::normalize(candidate->point->position - (pA->point->position + pB->point->position) * 0.5f);

    return acos(glm::dot(edge, toCandidate));  // Angle in radians
}

BallPivoting::~BallPivoting()
{
}
