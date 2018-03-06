#pragma once
#include "stdafx.h"
#include "SL_Socket.h"
#include "SL_Config.h"
#include "SL_BasicStructs.h"

class COM
{
public:
	enum INNER{ main, mgcp, sip, ann, cnf, prx, dtmf, sql, control, maxIN };
	enum OUTER{ mgcp_, sip_, maxOUT };

	static void Init();
	static SHP_SOCK GS(OUTER);//GetSOCK
	static void SignalInIO(INNER, string);

	static SHP_IO outerIO;
	static vector<boost::signals2::signal<void(string)>> vecSigsIN;
	static vector<boost::signals2::signal<void()>> vecSigsOUT;

private:

	static vector<SHP_SOCK> socketsOUT;

};