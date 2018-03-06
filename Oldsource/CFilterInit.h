#pragma once
#ifdef WIN32
#include "stdafx.h"
#endif
#ifdef linux
#include "stdinclude.h"
#endif
#include "Logger.h"
#include "Functions.h"
#include "Structs.h"
#include "ConfPoint.h"


using namespace std;
using namespace std::chrono;
extern string DateStr;
extern Logger* CLogger;

class CFilterInit
{
public:
	CFilterInit(){}
	CFilterInit(vector<SHP_CConfPoint>Caller, int ID);
	~CFilterInit(){ FreeSockFFmpeg(); }

	Initing data;
	void FreeSockFFmpeg();
private:
	void loggit(string a);
	int init_filter_graph(int ForClient);

	int tracks = 0;
	vector<SHP_CConfPoint>Caller_;
	int ID_;
};
typedef std::shared_ptr<CFilterInit> SHP_CFilterInit;