#pragma once
#include "SL_All.h"
#include "MScnf_Caller.h"


namespace cnf
{
	class Filter
	{
	public:
		Filter(vector<SHP_Caller>);

		FFF data;

	private:
		/*main activity*/
		int InitFilterGraph(int);

		/*Data*/
		int tracks;
		vector<SHP_Caller> vecCallers;
	};
	typedef shared_ptr<Filter> SHP_Filter;
}