#include "Octree.h"

void Octree::Insert(OctreeNode* node, E57Point* p, int depth)
{
    if (depth >= maxDepth) {
        node->points.push_back(p);
        return;
    }

    int index = (p->position.x > node->center.position.x) | ((p->position.y > node->center.position.y) << 1) | ((p->position.z > node->center.position.z) << 2);
    if (node->children[index] == nullptr) {
        double halfSize = node->size / 2;
        E57Point newCenter = {
            {
                node->center.position.x + halfSize * ((index & 1) ? 0.5 : -0.5),
                node->center.position.y + halfSize * ((index & 2) ? 0.5 : -0.5),
                node->center.position.z + halfSize * ((index & 4) ? 0.5 : -0.5)
            },
            glm::vec3(0), false
        };
        node->children[index] = new OctreeNode(newCenter, halfSize);
    }
    Insert(node->children[index], p, depth + 1);
}

void Octree::Clear(OctreeNode* node)
{
    for (int i = 0; i < 8; ++i)
    {
        if (node->children[i])
        {
            Clear(node->children[i]);
        }
    }
    delete node;
    node = nullptr;
}

OctreeNode*& Octree::GetRoot()
{
    return this->root;
}

Octree::Octree(E57Point center, float size, int depth)
{
    this->maxDepth = depth;
	root = new OctreeNode(center, size);
}

void Octree::Insert(E57Point* p)
{
	Insert(root, p, 0);
}

Octree::~Octree()
{
    Clear(root);
}
