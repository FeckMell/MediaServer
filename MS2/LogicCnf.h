#pragma once
#include "SL_All.h"
#include "LogicCnfPoint.h"
#include "LogicCnfRoom.h"

class LogicCnf
{
public:

	LogicCnf();

	map<string, boost::any> RegisterMePlease();

	void ProceedMGCP(/*params*/);
	void ProceedIPL(/*params*/);
	void ProceedCTR(/*params*/);

private:

	void MGCP_CRCX(/**/);
	void MGCP_MDCX(/**/);
	void MGCP_DLCX(/**/);

	map<string, SHP_LogicCnfPoint> _mapcnfpoints;
	map<string, SHP_LogicCnfRoom> _mapcnfrooms;

	static string _modulname;

};
string LogicCnf::_modulname = "LogicCnf";