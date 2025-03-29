#include "KDTree.h"

void KDTree::Insert(E57Point* point)
{
	KDTreeNode* node = new KDTreeNode;
	node->left = node->right = node->parent = nullptr;
	node->point = point;

	InsertNode(root, node, 0);
	size++;
}

int KDTree::GetSize()
{
	return this->size;
}

void KDTree::InsertNode(KDTreeNode*& node, KDTreeNode* newNode, int depth)
{
	KDTreeNode** current = &node;
	KDTreeNode* parent = nullptr;
	while (*current != nullptr)
	{
		parent = *current;

		unsigned cd = depth % k;
		float currentHelp = cd == 0 ? (*current)->point->position.x : cd == 1 ? (*current)->point->position.y : (*current)->point->position.z;
		float newNodeHelp = cd == 0 ? newNode->point->position.x : cd == 1 ? newNode->point->position.y : newNode->point->position.z;

		if (newNodeHelp < currentHelp) {
			current = &(*current)->left;
		}
		else {
			current = &(*current)->right;
		}
		depth++;
	}
	*current = newNode;
	newNode->parent = parent;
	if (depth > maxDepth)
		maxDepth = depth;
}

std::vector<KDTreeNode*> KDTree::GetNeighborsWithinRadius(KDTreeNode* queryNode, float radius) {

	std::vector<KDTreeNode*> neighbors;

	std::function<void(KDTreeNode*, int)> searchFunc = [&](KDTreeNode* node, int depth) {
		if (node == nullptr) return;

		float dist = glm::length(node->point->position - queryNode->point->position);

		if (dist <= radius) {
			neighbors.push_back(node);
		}

		int cd = depth % k;  // Splitting dimension
		float diff = queryNode->point->position[cd] - node->point->position[cd];
		float diff2 = glm::length(queryNode->point->position[cd] - node->point->position[cd]);

		// Recursively search left or right subtree based on distance
		if (diff < 0) {
			searchFunc(node->left, depth + 1);
			if (diff2 <= radius) {
				searchFunc(node->right, depth + 1);
			}
		}
		else {
			searchFunc(node->right, depth + 1);
			if (diff2 <= radius) {
				searchFunc(node->left, depth + 1);
			}
		}
		};

	searchFunc(root, 0);
	return neighbors;
}

std::vector<KDTreeNode*> KDTree::GetNeighborsOnRadius(glm::vec3 ballCenter, float radius, float tolerance)
{
	std::vector<KDTreeNode*> neighbors;

	std::function<void(KDTreeNode*, int)> searchFunc = [&](KDTreeNode* node, int depth) {
		if (node == nullptr) return;

		
		float dist = glm::length(node->point->position - ballCenter);

		float minDist = (radius - tolerance);
		float maxDist = (radius + tolerance);

		if (dist >= minDist && dist <= maxDist) {
			neighbors.push_back(node);
		}

		int cd = depth % k;  // Splitting dimension
		float diff = ballCenter[cd] - node->point->position[cd];
		float diff2 = glm::length(ballCenter[cd] - node->point->position[cd]);
		// Recursively search left or right subtree based on distance
		if (diff < 0) {
			searchFunc(node->left, depth + 1);
			if (diff2 <= maxDist) {
				searchFunc(node->right, depth + 1);
			}
		}
		else {
			searchFunc(node->right, depth + 1);
			if (diff2 <= maxDist) {
				searchFunc(node->left, depth + 1);
			}
		}
		};

	searchFunc(root, 0);
	return neighbors;
}

bool KDTree::ContainsPointsWithinRadiusBesidesPoints(KDTreeNode* queryNode, std::vector<E57Point*>& points, float radius)
{
	std::function<bool(KDTreeNode*, int)> searchFunc = [&](KDTreeNode * node, int depth) {
		if (node == nullptr) return false;

		float dist = glm::length(node->point->position - queryNode->point->position);

		if (dist <= radius) {
			for (E57Point* point : points)
			{
				if (point != queryNode->point)
					return true;
			}
		}

		int cd = depth % k;  // Splitting dimension
		float diff = queryNode->point->position[cd] - node->point->position[cd];
		float diff2 = glm::length(queryNode->point->position[cd] - node->point->position[cd]);

		// Recursively search left or right subtree based on distance
		if (diff < 0) {
			if (searchFunc(node->left, depth + 1))
				return true;
			if (diff2 <= radius) {
				if (searchFunc(node->right, depth + 1))
					return true;
			}
		}
		else {
			if (searchFunc(node->right, depth + 1))
				return true;
			if (diff2 <= radius) {
				if (searchFunc(node->left, depth + 1))
					return true;
			}
		}
		return false;
		};

	return searchFunc(root, 0);
}

KDTree::KDTree()
{
	root = nullptr;
}

void KDTree::DeleteNode(KDTreeNode*& node)
{
	delete node;
	node = nullptr;
}

KDTreeNode* KDTree::GetRoot()
{
	return root;
}

KDTreeNode* KDTree::GetRandomNode()
{
	int depth = rand() % (this->maxDepth + 1);
	KDTreeNode* node = root;
	for (int i = 0; i < depth; ++i)
	{
		if (rand() % 2 > 0)
		{
			if (node->left == nullptr)
				break;
			node = node->left;
		}
		else
		{
			if (node->right == nullptr)
				break;
			node = node->right;
		}
	}
	return node;
}

void KDTree::Clear()
{
	if (root == nullptr) 
		return;
	
	KDTreeNode* current = root;
	KDTreeNode* lastVisited = nullptr;

	while (current != nullptr) {
		if (current->left != nullptr && lastVisited != current->left) {
			current = current->left;
		}
		else {
			if (lastVisited != current) {
				if (current->right == nullptr) {
					DeleteNode(current);
					lastVisited = current; 
					current = nullptr; 
				}
				else {
					current = current->right;
				}
			}
			else {
				while (current != nullptr && (current->right == nullptr || current->right == lastVisited)) {
					lastVisited = current;
					current = current->parent; 
				}
				if (current != nullptr) {
					current = current->right;
				}
			}
		}
	}

	DeleteNode(root);
	root = nullptr; 
}

KDTreeNode* KDTree::FindNode(E57Point* point) {
	std::function<KDTreeNode* (KDTreeNode*, int)> searchFunc = [&](KDTreeNode* node, int depth) -> KDTreeNode* {
		if (node == nullptr) {
			return nullptr; 
		}

		if (node->point->position == point->position) {
			return node;
		}

		int cd = depth % k;
		float pointValue = point->position[cd];
		float nodeValue = node->point->position[cd];

		if (pointValue < nodeValue) {
			return searchFunc(node->left, depth + 1);
		}
		else {
			return searchFunc(node->right, depth + 1);
		}
		};

	return searchFunc(root, 0); 
}

KDTree::~KDTree()
{
	Clear();
}