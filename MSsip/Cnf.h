#pragma once
#include "stdafx.h"
#include "Point.h"

class Cnf
{
public:
	Cnf(SHP_Point);

	SHP_Point FindPoint(string);
	void AddPoint(SHP_Point);
	bool RmPoint(SHP_Point);

	string roomID;
	bool state = false;
private:
	vector<SHP_Point> vecPoints;
};
typedef shared_ptr<Cnf> SHP_Cnf;