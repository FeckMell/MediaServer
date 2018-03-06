#pragma once
#include "stdafx.h"

class MGCP
{
public:

	MGCP(string request_);

	void Reply();

	string Param(string name_);
	string PrintAll();
	void SetParam(string name_, string value_);

	SHP_SDP GetSDP(string which_);
	void SetSDP(string which_, SHP_SDP newSDP_);

private:

	string ReserveEventID(); //on crcx *@$
	void FreeEventID();      //on dlcx *@n

	SHP_SDP _clientsdp;
	SHP_SDP _serversdp;
	map<string, string> _data;

	static vector<int> _veceventids;

};
vector<int> MGCP::_veceventids;
typedef shared_ptr<MGCP> SHP_MGCP;