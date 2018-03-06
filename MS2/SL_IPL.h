#pragma once
#include "stdafx.h"

class IPL
{
public:

	IPL(string request_);
	
	string Param(string name_);
	string PrintAll();

private:

	map<string, string> _data;

};
typedef shared_ptr<IPL> SHP_IPL;