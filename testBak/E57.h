#pragma once
#include <E57Format/E57SimpleReader.h>
#include <Eigen/Dense> 

#include "E57Point.h"
#include "KDTree.h"

struct NormilizedPointsInfo {
	float minX, maxX, minY, maxY, minZ, maxZ;
};

class E57
{
private:
	KDTree tree;
	std::vector<E57Point> points;

	bool hasNormals;
	int count;
	NormilizedPointsInfo info;

public:
	void SetUpTree();
	E57(e57::ustring path);

	int ReadFile(e57::ustring & path);
	std::vector<E57Point>& getPoints();
	KDTree& getTree();

	void CalculateNormals();
	bool GetHasNormals();

	int getCount();
	NormilizedPointsInfo& getInfo();
	
	~E57();
};

