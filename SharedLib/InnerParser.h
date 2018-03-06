#pragma once
#include "stdafx.h"
#include "Functions.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class IPL
{
public:
	/*Main public activity*/
	IPL(const char* , EP);
	string PrintAll();

	/*Data storage*/
	string error = "";
	string ipl;
	EP sender;
	map<string, string> data;
	map<int, string> params;
private:
	/*Parsing functions*/
	void ParseMain();
};
typedef shared_ptr<IPL> SHP_IPL;