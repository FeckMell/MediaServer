#pragma once
#include "../../SharedLib/All.h"
#include "FCnf_Point.h"

namespace NFCnf
{
	class Filter
	{
	public:
		Filter(vector<SHP_Point> vecPoint);

		FFF data;

	private:
		/*main activity*/
		int InitFilterGraph(int which_);

		/*Data*/
		int tracks;
		vector<SHP_Point> vecPoint;
	};
	typedef shared_ptr<Filter> SHP_Filter;
}