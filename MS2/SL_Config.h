#pragma once
#include "stdafx.h"

class CFG
{
public:

	static void Init(string path_);

	static string Param(string name_);
	static string PtintAll();

private:

	static map<string, string> _data;

};