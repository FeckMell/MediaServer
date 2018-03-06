#pragma once
#include "stdafx.h"
#include "Functions.h"

extern string DateStr;
class SIP
{
public:
	//func
	enum Eventor { INVITE, ACK, BYE, Ringing, OK };
	SIP(string);
	SIP() {}
	std::string ResponseINVITE(int code, string end);
	std::string ResponseOK(int code, string end);
	std::string ResponseBAD(int code, string message);
	std::string ResponseRING(int code, string mess);

	void Parse(bool);

	int error;
	char mes[2049];
	udp::endpoint sender;

	//data
	std::string sip;
	Eventor CMD;//
	std::string SDP;

	std::string branch;//ответ с ним же
	std::string IP;
	std::string port;
	std::string maxForwards; //- номер транзакции, в ответе она -1
	std::string CseqNum; // номер, +1
	std::string CseqCMD; //
	std::string CallID;
	std::string tag;

private:

	void P_CMD();
	void Remove();
	void P_MaxForwards();
	void P_CallID();
	void P_seq();
	void P_tag();
	void P_branch();
};

class MGCP
{
public:
	//func
	MGCP(string);
	MGCP() {}
	string ResponseOK(int code, string end);
	std::string ResponseBAD(int code, string message);
	void Parse(bool);

	enum Eventor {ann, cnf, prx};
	int error;
	char mes[2049];
	udp::endpoint sender;

	//data
	std::string mgcp;
	std::string CMD;//
	Eventor Event;
	std::string EventS; //cnf/,ann/,prx/
	std::string EventNum;//cnf/n
	std::string EventEx;//cnf/1@[10.77.7.19]
	std::string paramM;
	std::string paramL;
	std::string paramI;
	std::string paramZ;
	std::string paramC;
	std::string paramS;
	std::string MessNum;
	std::string SDP;
private:
	
	void parseCMD();
	void EventP();
	void ParamM();
	void ParamC();
	void Remove();
};
typedef shared_ptr<MGCP> SHP_MGCP;