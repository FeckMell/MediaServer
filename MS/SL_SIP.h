#pragma once
#include "stdafx.h"
#include "SL_Communications.h"
#include "SL_LOG.h"
#include "SL_SDP.h"

class SIP
{
public:
	SIP(bool);

	void ReplyClient();

	SHP_SDP clientSDP;
	SHP_SDP serverSDP;
	string outerError = "";
	string innerError = "";

	map<string, string> data;

private:

	void Remove(string&);
	void SplitSIPandSDP(string&);
	void ParseMain();
	void Check();

	string ReplyRinging();
	string ReplyOK();
	void SendClient(string);

	EP sender;
	string sip = "";

};
typedef shared_ptr<SIP> SHP_SIP;