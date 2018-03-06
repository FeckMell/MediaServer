#pragma once
#include "stdafx.h"
#include "MGCPparser.h"
#include "Point.h"
#include "Ann.h"
#include "Cnf.h"

extern SHP_STARTUP init_Params;
extern SHP_NETDATA net_Data;

class MGCPcontrol
{
public:

	MGCPcontrol();
	void Preprocessing(SHP_MGCP);

private:

	void CRCX_CNF(SHP_MGCP);
	void CRCX_ANN(SHP_MGCP);

	void MDCX_CNF(SHP_MGCP);
	void RQNT_ANN(SHP_MGCP);

	void DLCX_CNF(SHP_MGCP);
	void DLCX_ANN(SHP_MGCP);

	SHP_Point FindPoint(string);
	void RemovePoint(SHP_Point);
	SHP_Cnf FindCnf(string);
	void RemoveCnf(SHP_Cnf);// delete ID
	SHP_Ann FindAnn(string);
	void RemoveAnn(SHP_Ann);// delete ID

	string GenSDP(string, SHP_MGCP);

	string ReservePort();
	void FreePort(string);

	string ReserveEventID();
	void FreeEventID(string);

	vector<int> usedPorts;
	vector<int> usedEventID;
	int lastSDP_ID;

	vector<SHP_Point> vecPoints;
	vector<SHP_Ann> vecAnns;
	vector<SHP_Cnf> vecCnfs;
};
typedef shared_ptr<MGCPcontrol> SHP_MGCPcontrol;