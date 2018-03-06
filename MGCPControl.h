#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "Logger.h"
#include "Parser.h"
#include "MGCPserver.h"
#include "ConfRoom.h"
#include "Ann.h"
#include "Proxy.h"
extern Logger CLogger;
extern string DateStr;
extern short int RTPport;

class CMGCPServer;
class MGCPControl
{
public:
	MGCPControl(CMGCPServer* s, string IP):server(s), my_IP(IP) {}
	std::string my_IP;
	CMGCPServer* server;

	void proceedCRCX(MGCP &mgcp);
	void proceedMDCX(MGCP &mgcp);
	void proceedRQNT(MGCP &mgcp);
	void proceedDLCX(MGCP &mgcp);
private:
	void loggit(string a);

	int SetRoomID();
	SHP_CConfRoom CreateNewRoom();

	int GetFreePort();
	void SetFreePort(int port);
	
	SHP_CConfRoom FindConf(string ID);
	SHP_Ann FindAnn(string ID);
	SHP_Proxy FindProxy(string ID);


	std::string GenSDP(int Port, MGCP &mgcp);
	int SDPFindMode(string SDP);


	std::vector<SHP_CConfRoom> RoomsVec_; // вектор конференций
	std::vector<SHP_Ann> AnnVec_;// вектор аннонсментов
	std::vector<SHP_Proxy> ProxyVec_;// вектор прокси

	std::vector<int> PortsinUse_; // вектор занятых портов
	std::vector<int> RoomsID_; // вектор ID
	

	std::mutex  mutex_;
	
};

typedef std::shared_ptr<MGCPControl> SHP_MGCPControl;