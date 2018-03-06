#pragma once
#include "stdafx.h"
using namespace std;

struct IPar
{
	//Tipes of parametrs.
	enum ParamNames
	{
		outerIP, innerIP, innerPort, logLevel, outerPort, rtpPort, maxTimeAnn, maxTimeCnf, maxTimePrx,
		mediaPath, homePath,
		maxParamNames
	};
	//Methods for Data.
	IPar(char*);
	string GetParams();
	void GetPathExe(char*);
	void ParseConfigFile();


	//Data.
	vector<string> data;
	string error = "";
};
typedef shared_ptr<IPar> SHP_IPar;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogsInit();