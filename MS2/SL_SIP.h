#pragma once
#include "stdafx.h"
#include "SL_SDP.h"

class SIP
{
public:

	SIP(string request_, EP endpoint_);

	string Param(string name_);
	SHP_SDP GetSDP(string which_);
	EP GetEndPoint();

	void SetSDP(SHP_SDP sdp_, string which_);
	void ReplyToClient();

private:

	SHP_SDP _clientsdp;
	SHP_SDP _serversdp;
	map<string, string> _data;

};
typedef shared_ptr<SIP> SHP_SIP;