#pragma once
#include "stdafx.h"

class SDP
{
public:

	SDP(string in_sdp_);
	SDP(string port_, string call_id_);

	string Param(string name_); /*+callid(sdp_id)*/
	string PrintAll();

private:

	map<string, string> _data;

	static int _sdpID;
};
typedef shared_ptr<SDP> SHP_SDP;