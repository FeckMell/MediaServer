#pragma once
#include "../../stdafx.h"
#include "../Basic/Functions.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class IPL
{
public:
	/*Main public activity*/
	IPL(string request_);

	string Param(string name_);
	string PrintAll();

	static string Generate(map<string, string> data_);

private:
	
	void ParseMain();

	map<string, string> mapData;

};
typedef shared_ptr<IPL> SHP_IPL;