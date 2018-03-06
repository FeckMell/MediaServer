#pragma once
#include "stdafx.h"
#include "SL_Functions.h"
#include "SL_Config.h"

class SDP
{
public:
	SDP(){}
	SDP(string);
	SDP(string, string);//GENERATE

	void ChangeModeS(string);
	static SDP Generate(string, string);

	string sdp = "";
	map<string, string> data;
	string error = "";
private:
	void Parse();
	void Check();

	static int sdpID;
};
typedef shared_ptr<SDP> SHP_SDP;