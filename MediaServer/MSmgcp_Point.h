#pragma once
#include "stdafx.h"
#include "SharedLib.h"
#include "MSmgcp_MGCPparser.h"


namespace mgcp
{
	class Point
	{
	public:

		Point(SHP_MGCP);
		void ModifyPoint(SHP_MGCP);

		SHP_MGCP mgcp;
		SHP_SDP serverSDP;
		SHP_SDP clientSDP;
		string callID;
		string eventID;

		bool state = false;//true - sendrecv, false - inactive

	};
	typedef shared_ptr<Point> SHP_Point;
}