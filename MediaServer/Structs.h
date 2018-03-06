#pragma once
#include "stdafx.h"
using namespace std;


//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
struct IPar
{
	//Tipes of parametrs.
	enum ParamNames
	{
		RTPPort, outerPort, innerPort, MediaPath, outerIP, innerIP, MaxTimeAnn, MaxTimeCnf, MaxTimePrx,
		LogLevel, PathEXE,
		MaxParamNames
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