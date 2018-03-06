#pragma once
#include "stdafx.h"

class MGCP
{
public:

	MGCP(string request_);

	string Param(string name_);
	string PrintAll();

private:

	map<string, string> _data;

};
typedef shared_ptr<MGCP> SHP_MGCP;