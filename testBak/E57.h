#pragma once
#include <E57Format/E57SimpleReader.h>

class E57
{
private:
	std::vector<float> points;
	int count;
public:
	E57(e57::ustring path);

	int ReadFile(e57::ustring & path);
	std::vector<float> getPoints();
	int getCount();

	~E57();
};

