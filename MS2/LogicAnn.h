#pragma once
#include "SL_All.h"
#include "LogicAnnPoint.h"

class LogicAnn
{
public:

	LogicAnn();

	map<string, boost::any> RegisterMePlease();

	void ProceedMGCP(/*params*/);
	void ProceedIPL(/*params*/);
	void ProceedCTR(/*params*/);

private:

	void MGCP_CRCX(/**/);
	void MGCP_RQNT(/**/);
	void MGCP_DLCX(/**/);
	void MGCP_NTFY(/**/);

	map<string, SHP_LogicAnnPoint> _mapannpoints;

	static string _modulname;

};
string LogicAnn::_modulname = "LogicAnn";