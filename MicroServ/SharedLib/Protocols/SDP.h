#pragma once
#include "../../stdafx.h"
#include "../Basic/Functions.h"
#include "../Interfaces/Config.h"

class SDP
{
public:
	SDP();
	SDP(string request_);
	SDP(string call_id_, string s_port_);//GENERATE

	void ChangeModeS(string new_mode_);

	string Param(string name_);
	string SetParam(string name_, string value_);
	string PrintAll();

private:

	void Parse();
	void Check();

	map<string, string> mapData;
	static int sdpID;

}; typedef shared_ptr<SDP> SHP_SDP;