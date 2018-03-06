#pragma once
#include "stdafx.h"

//namespace CFG
class CFG
{
public:
	//Methods for Data.
	static void Init(char*);
	static string GetParams();
	static void GetPathEXE(string);
	static void ParseConfigFile();

	//Data.
	static string version;
	static map<string, string> data;
	static string error;
	static string slash;
};