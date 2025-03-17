#pragma once
#include <E57Format/E57SimpleReader.h>

struct NormilizedPointsInfo {
	float minX, maxX, minY, maxY, minZ, maxZ;
};

class E57
{
private:
	std::vector<float> points;
	std::vector<float> normals;
	bool hasNormals;
	int count;
	NormilizedPointsInfo info;
public:
	E57(e57::ustring path);

	int ReadFile(e57::ustring & path);
	std::vector<float>& getPoints();
	int getCount();
	NormilizedPointsInfo& getInfo();
	~E57();
};

