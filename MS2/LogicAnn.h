#pragma once
#include "SL_All.h"
#include "LogicAnnPoint.h"

class LogicAnn
{
public:

	LogicAnn();

	map<string, boost::any> RegisterMePlease();

	void ProceedMGCP(SHP_MGCP mgcp_);
	void ProceedIPL(/*params*/);
	void ProceedCTRL(/*params*/);

private:

	void MGCP_CRCX(SHP_MGCP mgcp_);
	void MGCP_RQNT(SHP_MGCP mgcp_);
	void MGCP_DLCX(SHP_MGCP mgcp_);
	void MGCP_NTFY(/**/);

	map<string, SHP_LogicAnnPoint> _mapannpoints;

	static string _modulname;

};
string LogicAnn::_modulname = "LogicAnn";