#pragma once
#include "SL_All.h"

class LogicCnfPoint
{
public:

	LogicCnfPoint(SHP_SDP s_SDP_);

	void ModifyClientSDP(SHP_SDP c_SDP_);

	string PrintAll();
	string Param(string name_);
	SHP_SDP GetSDP(string which_);
	SHP_CALLER GetCaller();

private:

	SHP_SDP _serverSDP;
	SHP_SDP _clientSDP;

	map<string, string> _data;
	SHP_CALLER _basecaller;

};
typedef shared_ptr<LogicCnfPoint> SHP_LogicCnfPoint;