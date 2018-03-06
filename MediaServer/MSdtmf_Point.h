#pragma once
#include "stdafx.h"
#include "SharedLib.h"


namespace dtmf
{
	class Point
	{
	public:
		Point(SHP_IPL);
		void Run();
		~Point();

		string eventID;
		string callID;
		bool state = true;
	private:

		void RunIO();
		void Receive(boost::system::error_code, size_t);
		
		bool Analyze(uint8_t ch[2]);
		void SendModul();

		SHP_thread th;
		EP endPoint;
		SHP_SOCK socket;
		Data rawBuf;

		string serverPort;
		string clientPort;
		string clientIP;

		vector<int> buttons;

	};
	typedef shared_ptr<Point> SHP_Point;
}