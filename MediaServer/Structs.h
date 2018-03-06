#pragma once
#include "../SharedSource/stdafx.h"
using namespace std;

struct STARTUP
{
	//Tipes of parametrs.
	enum ParamNames
	{
		outerIP, innerIP, innerPort, logLevel, outerPort, rtpPort, maxTimeAnn, maxTimeCnf, maxTimePrx,
		mediaPath, homePath, portSIP,
		maxParamNames
	};
	//Methods for Data.
	STARTUP(char*);
	string GetParams();
	void GetPathExe(char*);
	void ParseConfigFile();


	//Data.
	vector<string> data;
	string error = "";
};
typedef shared_ptr<STARTUP> SHP_STARTUP;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//void LogsInit(string my_modul_name_);