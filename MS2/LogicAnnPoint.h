#pragma once
#include "SL_ALL.h"

class LogicAnnPoint
{
public:

	LogicAnnPoint(SHP_SDP c_SDP_, SHP_SDP s_SDP_);
	~LogicAnnPoint();

	void RequestMusic(string filename_, string loop_);
	void StopMusic();

	string Param(string name_);
	string PrintAll();

	SHP_CALLER GetCaller();

private:

	void CheckFileExistance();
	void NotifyModul(string event_);

	map<string, string> _data;
	SHP_CALLER _basecaller;

};
typedef shared_ptr<LogicAnnPoint> SHP_LogicAnnPoint;