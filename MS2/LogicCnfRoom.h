#pragma once
#include "SL_All.h"
#include "LogicCnfPoint.h"

class LogicCnfRoom
{
public:

	LogicCnfRoom(SHP_LogicCnfPoint point_, string roomnum_);

	void AddPoint(SHP_LogicCnfPoint point_);
	void DelPoint(SHP_LogicCnfPoint point_);

	bool IsEmpty();

	string Param(string name_);
	string PrintAll();

private:

	void Process();
	void NotifyModul(string event_);

	map<string, string> _data;
	map<string, SHP_LogicCnfPoint> _mapcnfpoints;

};
typedef shared_ptr<LogicCnfRoom> SHP_LogicCnfRoom;