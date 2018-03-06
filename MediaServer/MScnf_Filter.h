#pragma once
#include "stdafx.h"
#include "SharedLib.h"
#include "MScnf_CnfPoint.h"


namespace cnf
{
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
}