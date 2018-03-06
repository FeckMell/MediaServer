#pragma once
#include "stdafx.h"
#include "CnfPoint.h"

extern SHP_STARTUP init_Params;

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