#pragma once
#include "stdafx.h"
#include "Config.h"
#include "BasicStructs.h"

//namespace NET
class NET
{
public:
	enum INNER{ main, mgcp_i, sip_i, ann, cnf, prx, dtmf, sql, maxIN };
	enum OUTER{ mgcp, sip, maxOUT };

	static void Init();
	static SHP_SOCK GS(INNER);//GetSOCK
	static SHP_SOCK GS(OUTER);//GetSOCK

	static EP GE(INNER);//GetEndpPoint

	static IO& GI(INNER);//GetIO
	static IO& GI(OUTER);//GetIO

	static void SendModul(INNER, INNER, string);
	static void RunIO(INNER);
	static void RunIO(OUTER);

	static vector<SHP_SOCK> socketsIN;
	static vector<SHP_SOCK> socketsOUT;
	static vector<EP> endPointsIN;
	static vector<SHP_IO> iosIN;
	static vector<SHP_IO> iosOUT;
};