#pragma once
#ifdef WIN32
#include "stdafx.h"
#endif
#ifdef linux
#include "stdinclude.h"
#endif
#include "Structs.h"
#include "Functions.h"
#include "Logger.h"
#include "Parser.h"
#include "MGCPserver.h"
#include "ConfRoom.h"
#include "Ann.h"
#include "Proxy.h"
extern Logger* CLogger;
extern string DateStr;
extern short int RTPport;

class CMGCPServer;
class MGCPControl
{
public:
	std::string my_IP;
	CMGCPServer* server;

	void proceedCRCX(MGCP &mgcp);
	void proceedMDCX(MGCP &mgcp);
	void proceedRQNT(MGCP &mgcp);
	void proceedDLCX(MGCP &mgcp);

	int GetFreePort();
	void SetFreePort(int port);
private:
	void loggit(string a);

	int SetRoomID();
	SHP_CConfRoom CreateNewRoom();

	
	
	SHP_CConfRoom FindConf(string ID);
	SHP_Ann FindAnn(string ID);
	SHP_Proxy FindProxy(string ID);


	std::string GenSDP(int Port, MGCP &mgcp);
	int SDPFindMode(string SDP);


	std::vector<SHP_CConfRoom> RoomsVec_; // ������ �����������
	std::vector<SHP_Ann> AnnVec_;// ������ ������������
	std::vector<SHP_Proxy> ProxyVec_;// ������ ������

	std::vector<int> PortsinUse_; // ������ ������� ������
	std::vector<int> RoomsID_; // ������ ID
	

	std::mutex  mutex_;
	
};

typedef std::shared_ptr<MGCPControl> SHP_MGCPControl;