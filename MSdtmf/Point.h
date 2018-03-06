#pragma once
#include "stdafx.h"
namespace dtmf
{
	class Point
	{
	public:
		Point(SHP_IPL);

		//void Receive(boost::system::error_code, size_t);
		bool Analyze(uint8_t ch[2]);

		Data rawBuf;

		string eventID;
		string callID;

		EP endPoint;
		SHP_SOCK socket;

	private:
		void SendModul();

		string serverPort;
		string clientPort;
		string clientIP;

		vector<int> buttons;
	};
	typedef shared_ptr<Point> SHP_Point;
}