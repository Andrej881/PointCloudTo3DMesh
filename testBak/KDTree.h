#pragma once
//for 3 dimensions

#include <functional>
#include <vector>

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

	void InsertNode(KDTreeNode*& node, KDTreeNode* newNode, int depth);
public:
	KDTree();
	std::vector<KDTreeNode*> GetNeighborsWithinRadius(KDTreeNode* queryNode, float radius);
	std::vector<KDTreeNode*> GetNeighborsOnRadius(KDTreeNode* queryNode, float radius);
	void DeleteNode(KDTreeNode *& node);
	void Insert(E57Point* point);
	KDTreeNode* GetRoot();
	void Clear();
	KDTreeNode* FindNode(E57Point* point);
	~KDTree();
};

