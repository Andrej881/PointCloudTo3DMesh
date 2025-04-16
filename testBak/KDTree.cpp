#include "KDTree.h"

void KDTree::Insert(E57Point* point)
{
	KDTreeNode* node = new KDTreeNode;
	node->left = node->right = node->parent = nullptr;
	node->point = point;

	InsertNode(root, node);
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

void KDTree::InsertNode(KDTreeNode*& node, KDTreeNode* newNode)
{
	int depth = 0;
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

std::vector<KDTreeNode*> KDTree::GetNeighborsWithinRadius(glm::vec3 position, float radius) {

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

		float dist2 = glm::distance2(node->point->position, position);
		if (dist2 <= radius2 && position != node->point->position) {
			neighbors.push_back(node);
		}

		int cd = depth % k;  // Rozdeæuj˙ca dimenzia
		float diff = position[cd] - node->point->position[cd];
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
	}

	return neighbors;
}

std::vector<KDTreeNode*> KDTree::GetKNearestNeighbors(glm::vec3 position, int k)
{
	
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
		if (node == nullptr || node->point->position == position) continue;

		float dist2 = glm::distance2(node->point->position, position);

		if ((int)maxHeap.size() < k) {
			maxHeap.push({ node, dist2 });
		}
		else if (dist2 < maxHeap.top().distance2) {
			maxHeap.pop();
			maxHeap.push({ node, dist2 });
		}

		int axis = depth % this->k;
		float diff = position[axis] - node->point->position[axis];
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

/*std::vector<KDTreeNode*> KDTree::GetNeighborsOnRadius(glm::vec3 ballCenter, float radius, float tolerance)
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
}*/

bool KDTree::ContainsPointsWithinRadiusBesidesPoints(glm::vec3 position, std::unordered_set<E57Point*>& points, float radius)
{
	float radius2 = radius * radius;
	if (root == nullptr) return false;

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

		float dist2 = glm::distance2(node->point->position, position);
		if (dist2 <= radius2) {

			if(points.count(node->point) <= 0)
			{
				return true;
			}
			/*bool differant = true;
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
			}*/
		}

		int cd = depth % k;  // Rozdeæuj˙ca dimenzia
		float diff = position[cd] - node->point->position[cd];
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
	}

	return false;
}

KDTree::KDTree()
{
	root = nullptr;
}

KDTreeNode* KDTree::BuildBalanced(std::vector<E57Point*>& points, int depth)
{	

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