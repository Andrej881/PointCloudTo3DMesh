#pragma once
#include "E57Point.h"
#include <vector>

struct OctreeNode {
    E57Point center;
    OctreeNode* children[8] = { nullptr };
    float size;
    float divergence, chi;
    std::vector<E57Point*> points;

    OctreeNode(E57Point c, float s) : center(c), size(s) {
        for (int i = 0; i < 8; ++i)
        {
            children[i] = nullptr;
        }
    }

    bool contains(E57Point* p) {
        return (p->position.x >= center.position.x - size / 2 && p->position.x <= center.position.x + size / 2 &&
            p->position.y >= center.position.y - size / 2 && p->position.y <= center.position.y + size / 2 &&
            p->position.z >= center.position.z - size / 2 && p->position.z <= center.position.z + size / 2);
    }

    void computeDivergence() {

        if (this->points.size() <= 0)
        {
            float totalDivergence = 0.0f;
            int validChildren = 0;
            for (OctreeNode*& child : children)
            {
                if (child != nullptr)
                {
                    child->computeDivergence();
                    totalDivergence += child->divergence;
                    validChildren++;
                }
            }
            if (validChildren > 0) {
                this->divergence = this->chi = totalDivergence / validChildren;
            }
            else
                this->divergence = this->chi = 0.0f;
        }
        else
        {
            float volume = size * size * size;
            float divN = 0.0f;
            for (const auto& p : this->points) {
                divN += glm::dot(p->normal, this->center.position - p->position);
                //divN += glm::dot(p->normal, p->position);
            }
            this->divergence = this->chi = divN / volume;
        }
            
    }
};

class Octree
{
private:
    OctreeNode* root;
    int maxDepth;
public:
    Octree(E57Point center, float size, int depth);

    void Insert(E57Point* p);
    void Insert(OctreeNode* node, E57Point* p, int depth);
    void Clear(OctreeNode* node);

    OctreeNode*& GetRoot();

    ~Octree();

    Octree() = default;
};

