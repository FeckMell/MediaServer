#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "Logger.h"
#include "Parser.h"
#include "MGCPserver.h"
extern Logger CLogger;
extern string DateStr;

class CMGCPServer;
class SIPControl
{
public:
	SIPControl(CMGCPServer* s, string IP) :server(s), my_IP(IP) {}
	std::string my_IP;
	CMGCPServer* server;

	void proceedINVITE(SIP &sip);
	void proceedACK(SIP &sip);
	void proceedRINGING(SIP &sip);
	void proceedBYE(SIP &sip);
	void proceedTRYING(SIP &sip);
	void proceedOK(SIP &sip);
	void proceedREFER(SIP &sip);
	void proceedSUBSCRIBE(SIP &sip);

private:
	void loggit(string a);

};

typedef std::shared_ptr<SIPControl> SHP_SIPControl;