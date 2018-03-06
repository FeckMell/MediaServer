#pragma once
#include "../../stdafx.h"

class CFG
{
public:

	static void Init(char* path_);
	static string Param(string name_);
	static string PrintAll();

private:

	static void GetPathEXE(string path_);
	static void ParseConfigFile();

	static map<string, string> mapData;

};