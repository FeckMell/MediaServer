#pragma once
#include "stdafx.h"

extern string DateStr;

class MGCP
{
public:
	//func
	MGCP(string req);
	string ResponseOK(int code, string end);
	std::string ResponseBAD(int code, string message);
	enum Eventor {ann, cnf, prx};

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
	int error;

private:
	void Parse();
	void parseCMD();
	void EventP();
	void ParamM();
	void ParamL();
	void ParamI();
	void ParamZ();
	void ParamC();
	void ParamS();
	void Remove();
	bool Valid();	
};
