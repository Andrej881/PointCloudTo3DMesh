#pragma once
//for 3 dimensions

#include <functional>
#include <vector>
#include <unordered_set>

#include "E57Point.h"

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

	void InsertNode(KDTreeNode*& node, KDTreeNode* newNode, int depth);
public:
	KDTree();
	std::vector<KDTreeNode*> GetNeighborsWithinToroidalRadius(const glm::vec3& A, const glm::vec3& B, float radius);
	std::vector<KDTreeNode*> GetNeighborsWithinRadius(KDTreeNode* queryNode, float radius);
	std::vector<KDTreeNode*> GetKNearestNeighbors(KDTreeNode* queryNode, int k);
	std::vector<KDTreeNode*> GetNeighborsOnRadius(glm::vec3 ballCenter,  float radius, float tolerance);
	bool ContainsPointsWithinRadiusBesidesPoints(KDTreeNode* queryNode, std::vector<E57Point*>& points, float radius);
	void DeleteNode(KDTreeNode *& node);
	void Insert(E57Point* point);
	int GetSize();
	KDTreeNode* GetRoot();
	KDTreeNode* GetRandomNode();
	void Clear();
	KDTreeNode* FindNode(E57Point* point);
	~KDTree();
};

