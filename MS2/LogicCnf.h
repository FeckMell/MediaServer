#pragma once
#include "SL_All.h"
#include "LogicCnfPoint.h"
#include "LogicCnfRoom.h"

class LogicCnf
{
public:

	LogicCnf();

	map<string, boost::any> RegisterMePlease();

	void ProceedMGCP(SHP_MGCP mgcp_);
	void ProceedIPL(/*params*/);
	void ProceedCTRL(/*params*/);

private:

	void MGCP_CRCX(SHP_MGCP mgcp_);
	void MGCP_MDCX(SHP_MGCP mgcp_);
	void MGCP_DLCX(SHP_MGCP mgcp_);

	map<string, SHP_LogicCnfPoint> _mapcnfpoints;
	map<string, SHP_LogicCnfRoom> _mapcnfrooms;

	static string _modulname;

};
string LogicCnf::_modulname = "LogicCnf";