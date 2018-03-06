#pragma once
#include "../SharedSource/stdafx.h"
#include "../SharedSource/Structs.h"
#include "../SharedSource/Functions.h"
#include "CnfPoint.h"

extern SHP_IPar init_Params;

class Filter
{
public:
	Filter(vector<SHP_CnfPoint>);

	FFF data;

private:
	/*main activity*/
	int InitFilterGraph(int);

	/*Data*/
	int tracks;
	vector<SHP_CnfPoint> cnfPoints;
};
typedef shared_ptr<Filter> SHP_Filter;