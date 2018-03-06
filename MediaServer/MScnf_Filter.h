#pragma once
#include "stdafx.h"
#include "SharedLib.h"
#include "MScnf_Point.h"


namespace cnf
{
	class Filter
	{
	public:
		Filter(vector<SHP_Point>);

		FFF data;

	private:
		/*main activity*/
		int InitFilterGraph(int);

		/*Data*/
		int tracks;
		vector<SHP_Point> vecPoints;
	};
	typedef shared_ptr<Filter> SHP_Filter;
}