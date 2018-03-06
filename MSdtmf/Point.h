#pragma once
#include "stdafx.h"

extern SHP_STARTUP init_Params;
extern SHP_NETDATA net_Data;

class Point
{
public:
	Point(SHP_IPL, IO&);

	bool Analyze(uint8_t ch[2]);

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