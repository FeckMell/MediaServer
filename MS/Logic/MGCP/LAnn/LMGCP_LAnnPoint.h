#pragma once
#include "../../../SharedLib/All.h"
namespace NLmgcpAnn
{
	class Point
	{
	public:
		Point(SHP_MGCP mgcp_);
		~Point();

		void RQNT(SHP_MGCP mgcp_);
		void DLCX();

		string Param(string name_);
	private:

		bool CheckFileExistance();
		void SendModul(string type_);

		map<string, string> mapData;
		SHP_Caller baseCaller;
	};
	typedef shared_ptr<Point> SHP_Point;
}