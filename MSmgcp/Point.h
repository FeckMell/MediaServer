#pragma once
#include "stdafx.h"
#include "MGCPparser.h"
namespace mgcp
{
	class Point
	{
	public:
		//enum State{ login, pass, ready, paused };
		Point(SHP_MGCP);
		void ModifyPoint(SHP_MGCP);

		string PrintPoint();

		string serverSDP;
		string clientSDP;
		string serverPort;
		string clientPort;
		string clientIP;
		string callID;

		string eventID;

		bool state = false;//true - sendrecv, false - inactive
	private:

		string FindSDPmode(string);
		bool ChangeSDPmode(string);
		string GetIPfromSDP(string);
		string GetPortFromSDP(string);
	};
	typedef shared_ptr<Point> SHP_Point;
}