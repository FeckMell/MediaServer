#pragma once
#include "../../../SharedLib/All.h"
namespace NLmgcpCnf
{
	class Point
	{
	public:

		Point(SHP_MGCP mgcp_);
		~Point();

		void ModPoint(SHP_MGCP mgcp_);

		string Param(string name_);
		string PrintAll();


	private:

		map<string, string> mapData;
		SHP_Caller baseCaller;

	};
	typedef shared_ptr<Point> SHP_Point;
}