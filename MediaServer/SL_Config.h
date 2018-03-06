#pragma once
#include "stdafx.h"

//namespace CFG
class CFG
{
public:
	enum ParamNames
	{
		outerIP, innerIP, innerPort, logLevel, mgcpPort, rtpPort, maxTimeAnn, maxTimeCnf, maxTimePrx,
		mediaPath, homePath, sipPort,
		maxParamNames
	};
	//Methods for Data.
	static void Init(char*);
	static string GetParams();
	static void GetPathEXE(string);
	static void ParseConfigFile();

	//Data.
	static vector<string> data;
	static string error;
};