#pragma once
#include <E57Format/E57SimpleReader.h>
#include <Eigen/Dense> 
#include <unordered_set>
#include <queue>
#include <unordered_map>
#include <thread>
#include <mutex>

#include "E57Point.h"
#include "KDTree.h"

struct NormilizedPointsInfo {
	float minX, maxX, minY, maxY, minZ, maxZ;
};

class E57
{
private:
	std::mutex mutex;
	KDTree tree;
	std::vector<E57Point> points;

	bool hasNormals;
	int count;
	NormilizedPointsInfo info;
	void OrientNormals(std::unordered_map<E57Point*, std::vector<KDTreeNode*>>& neighborsCache);

	void CalculateNormalsThread(std::unordered_map<E57Point*, std::vector<KDTreeNode*>>& neighborsCache, int startIndex, int endIndex, int numOfNeigbours, float radius);
	//void OrientNormalsThread(std::unordered_map<E57Point*, std::vector<KDTreeNode*>>& neighborsCache, int startIndex, int endIndex, int numOfNeigbours);
	bool stopCalulatingNormals = false;
	bool calculating = false;

public:
	void SetUpTree();
	E57(e57::ustring path);

	int ReadFile(e57::ustring & path);
	std::vector<E57Point>& getPoints();
	KDTree& getTree();

	void CalculateNormals(float radius, int numOfNeigbors);
	bool GetHasNormals();
	bool GetCalculating();
	void StopCalculatingNormals();

	int getCount();
	NormilizedPointsInfo& getInfo();
	
	~E57();
};

