#include "KDTree.h"

void KDTree::Insert(E57Point* point)
{
	KDTreeNode* node = new KDTreeNode;
	node->left = node->right = node->parent = nullptr;
	node->point = point;

	InsertNode(root, node, 0);
	size++;
}

void KDTree::InsertPoints(std::vector<E57Point*>& points)
{
	if(root != nullptr)
		Clear();
	root = BuildBalanced(points, 0);
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
	float radius2 = radius * radius;
	if (root == nullptr) return neighbors;

	// Vlastn˝ stack na heap (namiesto rekurzie alebo std::stack)
	struct StackFrame {
		KDTreeNode* node;
		int depth;
	};

	// Alok·cia stacku s dostatoËnou kapacitou (napr. 64 prvkov na zaËiatok)
	std::vector<StackFrame> stack;
	stack.reserve(1000); // Predalok·cia, aby sa zbytoËne nealokovalo Ëasto
	stack.push_back({ root, 0 });

	while (!stack.empty()) {
		StackFrame current = stack.back();
		stack.pop_back();

		KDTreeNode* node = current.node;
		int depth = current.depth;

		if (node == nullptr) continue;

		float dist2 = glm::distance2(node->point->position, queryNode->point->position);
		if (dist2 <= radius2) {
			neighbors.push_back(node);
		}

		int cd = depth % k;  // Rozdeæuj˙ca dimenzia
		float diff = queryNode->point->position[cd] - node->point->position[cd];
		float diff2 = std::abs(diff);

		// Rozhodnutie, ktorÈ vetvy prehæad·vaù
		if (diff < 0) {
			stack.push_back({ node->left, depth + 1 });
			if (diff2 <= radius) {
				stack.push_back({ node->right, depth + 1 });
			}
		}
		else {
			stack.push_back({ node->right, depth + 1 });
			if (diff2 <= radius) {
				stack.push_back({ node->left, depth + 1 });
			}
		}
		/*if (diff < 0) {
			// Check if the left child is within the bounding box
			if (queryNode->point->position[cd] - radius <= node->point->position[cd]) {
				stack.push_back({ node->left, depth + 1 });
			}
			// Check if the right child might intersect the radius bounding box
			if (diff2 <= radius) {
				stack.push_back({ node->right, depth + 1 });
			}
		}
		else {
			// Check if the right child is within the bounding box
			if (queryNode->point->position[cd] + radius >= node->point->position[cd]) {
				stack.push_back({ node->right, depth + 1 });
			}
			// Check if the left child might intersect the radius bounding box
			if (diff2 <= radius) {
				stack.push_back({ node->left, depth + 1 });
			}
		}*/
	}

	return neighbors;
}

std::vector<KDTreeNode*> KDTree::GetKNearestNeighbors(KDTreeNode* queryNode, int k)
{
	/*struct NodeDist {
		KDTreeNode* node;
		float distance2;
	};
	std::vector<NodeDist> distances;
	std::vector<KDTreeNode*> neighbors;

	if (root == nullptr) return neighbors;

	// Vlastn˝ stack na heap (namiesto rekurzie alebo std::stack)
	struct StackFrame {
		KDTreeNode* node;
		int depth;
	};

	// Alok·cia stacku s dostatoËnou kapacitou (napr. 64 prvkov na zaËiatok)
	std::vector<StackFrame> stack;
	stack.reserve(64); // Predalok·cia, aby sa zbytoËne nealokovalo Ëasto
	stack.push_back({ root, 0 });

	while (!stack.empty()) {
		StackFrame current = stack.back();
		stack.pop_back();

		KDTreeNode* node = current.node;
		int depth = current.depth;

		if (node == nullptr) continue;

		float dist2 = glm::distance2(node->point->position, queryNode->point->position);
		bool same = false;
		if (queryNode == node)
			same = true;
		if (!same)
		{
			for (int i = distances.size() - 1; i >= 0; i--)
			{
				if (node == distances[i].node)
				{
					same = true;
					break;
				}
			}
		}
		if (!same)
		{
			if (distances.size() == 0) {
				distances.push_back({ node , dist2 });
			}
			else
			{
				for (int i = distances.size() - 1; i >= 0; i--)
				{
					if (dist2 > distances[i].distance2 || i == 0)
					{
						int index = i + 1;
						if (index < k)
						{
							if (index == distances.size())
								distances.push_back({ node , dist2 });
							else
							{
								if (distances.size() < k)
									distances.push_back({nullptr, 1});
								for (int i = distances.size() - 1; i > index; i--)
								{
									distances[i] = distances[i - 1];
								}
								distances[index] = { node , dist2 };
							}

						}
						break;
					}
				}
			}
		}		

		int cd = depth % this->k;  // Rozdeæuj˙ca dimenzia
		float diff = queryNode->point->position[cd] - node->point->position[cd];
		float diff2 = diff * diff;

		float maxMinDist = distances.size() == k - 1 ? distances.back().distance2 : 1;

		// Rozhodnutie, ktorÈ vetvy prehæad·vaù
		if (diff < 0) {
			stack.push_back({ node->left, depth + 1 });
			if (diff2 <= maxMinDist) {
				stack.push_back({ node->right, depth + 1 });
			}
		}
		else {
			stack.push_back({ node->right, depth + 1 });
			if (diff2 <= maxMinDist) {
				stack.push_back({ node->left, depth + 1 });
			}
		}
	}

	for (int i = 0; i < distances.size(); ++i)
	{
		neighbors.push_back(distances[i].node);
	}

	return neighbors;*/
	struct Neighbor {
		KDTreeNode* node;
		float distance2;

		// Max-heap: higher distance = higher priority (for removal)
		bool operator<(const Neighbor& other) const {
			return distance2 < other.distance2;
		}
	};

	std::vector<KDTreeNode*> result;
	if (root == nullptr || k <= 0) return result;

	// Max-heap priority queue to keep k closest neighbors
	std::priority_queue<Neighbor> maxHeap;

	struct StackFrame {
		KDTreeNode* node;
		int depth;
	};

	std::vector<StackFrame> stack;
	stack.reserve(64);
	stack.push_back({ root, 0 });

	while (!stack.empty()) {
		StackFrame current = stack.back();
		stack.pop_back();

		KDTreeNode* node = current.node;
		int depth = current.depth;
		if (node == nullptr || node == queryNode) continue;

		float dist2 = glm::distance2(node->point->position, queryNode->point->position);

		if ((int)maxHeap.size() < k) {
			maxHeap.push({ node, dist2 });
		}
		else if (dist2 < maxHeap.top().distance2) {
			maxHeap.pop();
			maxHeap.push({ node, dist2 });
		}

		int axis = depth % this->k;
		float diff = queryNode->point->position[axis] - node->point->position[axis];
		float diff2 = diff * diff;

		KDTreeNode* first = diff < 0 ? node->left : node->right;
		KDTreeNode* second = diff < 0 ? node->right : node->left;

		stack.push_back({ first, depth + 1 });

		// Only check the other side if it might contain closer neighbors
		if ((int)maxHeap.size() < k || diff2 < maxHeap.top().distance2) {
			stack.push_back({ second, depth + 1 });
		}
	}

	// Collect results from heap (in reverse distance order)
	result.reserve(k);
	while (!maxHeap.empty()) {
		result.push_back(maxHeap.top().node);
		maxHeap.pop();
	}

	// Optional: reverse to return closest first
	std::reverse(result.begin(), result.end());

	return result;
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
		float diff2 = std::abs(ballCenter[cd] - node->point->position[cd]);
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
			bool differant = true;
			for (E57Point* point : points)
			{
				if (point == node->point)
				{
					differant = false;
					continue;
				}
			}
			if (differant)
			{
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

KDTreeNode* KDTree::BuildBalanced(std::vector<E57Point*>& points, int depth)
{
	/*if (points.empty()) return nullptr;

	struct NodeInfo {
		std::vector<E57Point*> points;
		int depth;
		KDTreeNode* parent;
		KDTreeNode** node; // Pointer to the node being constructed
	};

	std::stack<NodeInfo> stack;
	KDTreeNode* root = nullptr;

	// Start by adding the root node to the stack
	stack.push(NodeInfo{ points, 0, nullptr, &root });

	while (!stack.empty()) {
		NodeInfo current = stack.top();
		stack.pop();

		// Extract points and depth
		std::vector<E57Point*>& pointsToProcess = current.points;
		int depth = current.depth;
		KDTreeNode* parent = current.parent;
		KDTreeNode** currentNode = current.node;

		if (pointsToProcess.empty()) {
			continue;
		}

		int axis = depth % k;

		// Sort the points by the current axis
		std::sort(pointsToProcess.begin(), pointsToProcess.end(), [axis](E57Point* a, E57Point* b) {
			return a->position[axis] < b->position[axis];
			});

		// Find the median index
		int medianIndex = pointsToProcess.size() / 2;

		// Create the current node
		KDTreeNode* node = new KDTreeNode;
		node->point = pointsToProcess[medianIndex];
		node->parent = parent;
		*currentNode = node;  // Set the node pointer

		// Split the points into left and right subsets
		std::vector<E57Point*> leftPoints(pointsToProcess.begin(), pointsToProcess.begin() + medianIndex);
		std::vector<E57Point*> rightPoints(pointsToProcess.begin() + medianIndex + 1, pointsToProcess.end());

		// Push the left and right children to the stack, if necessary
		if (!leftPoints.empty()) {
			stack.push(NodeInfo{ leftPoints, depth + 1, node, &node->left });
		}
		if (!rightPoints.empty()) {
			stack.push(NodeInfo{ rightPoints, depth + 1, node, &node->right });
		}
	}

	return root;*/

	if (points.empty()) return nullptr;

	int axis = depth % k;

	std::sort(points.begin(), points.end(), [axis](E57Point* a, E57Point* b) {
		return a->position[axis] < b->position[axis];
		});

	int medianIndex = points.size() / 2;
	KDTreeNode* node = new KDTreeNode;
	node->point = points[medianIndex];
	node->parent = nullptr;

	std::vector<E57Point*> leftPoints(points.begin(), points.begin() + medianIndex);
	std::vector<E57Point*> rightPoints(points.begin() + medianIndex + 1, points.end());

	node->left = BuildBalanced(leftPoints, depth + 1);
	node->right = BuildBalanced(rightPoints, depth + 1);

	return node;
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
	
	size = 0;
	maxDepth = 0;
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