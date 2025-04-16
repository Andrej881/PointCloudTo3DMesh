#pragma once
//for 3 dimensions

#define GLM_ENABLE_EXPERIMENTAL
#include <functional>
#include <vector>
#include <unordered_set>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "E57Point.h"

#include <stack>
#include <queue>

struct KDTreeNode {
	E57Point* point;
	KDTreeNode * left;
	KDTreeNode * right;
	KDTreeNode * parent;	
};

class KDTree
{
private:
	KDTreeNode* root;
	const int k = 3;
	int maxDepth = 0;
	int size = 0;

	void InsertNode(KDTreeNode*& node, KDTreeNode* newNode);
	KDTreeNode* BuildBalanced(std::vector<E57Point*>& points, int depth);
public:
	KDTree();

	std::vector<KDTreeNode*> GetNeighborsWithinRadius(glm::vec3 position, float radius);
	std::vector<KDTreeNode*> GetKNearestNeighbors(glm::vec3 position, int k);
	std::vector<KDTreeNode*> GetNeighborsOnRadius(glm::vec3 ballCenter,  float radius, float tolerance);
	bool ContainsPointsWithinRadiusBesidesPoints(glm::vec3 position, std::unordered_set<E57Point*>& points, float radius);
	void DeleteNode(KDTreeNode *& node);
	void Insert(E57Point* point);
	void InsertPoints(std::vector<E57Point*>& points);
	int GetSize();
	KDTreeNode* GetRoot();
	KDTreeNode* GetRandomNode();
	void Clear();
	KDTreeNode* FindNode(E57Point* point);
	~KDTree();
};

