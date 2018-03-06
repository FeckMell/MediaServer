#pragma once
#include "stdafx.h"
#include "SharedLib.h"
#include "MSmgcp_MGCPparser.h"


namespace mgcp
{
	class Point
	{
	public:
		//enum State{ login, pass, ready, paused };
		Point(SHP_MGCP);
		void ModifyPoint(SHP_MGCP);

		//string PrintPoint();

		SHP_SDP serverSDP;
		SHP_SDP clientSDP;
		//string serverPort;
		//string clientPort;
		//string clientIP;
		string callID;

		string eventID;

		bool state = false;//true - sendrecv, false - inactive
	private:
	};
	typedef shared_ptr<Point> SHP_Point;
}