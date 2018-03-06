#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "Logger.h"
#include "Parser.h"
#include "MGCPserver.h"
#include "Conf.h"
#include "Ann.h"
extern Logger CLogger;

class CMGCPServer;
class ConfControl
{
public:
	std::string my_IP;
	CMGCPServer* server;

	void proceedCRCX(MGCP &mgcp, const udp::endpoint& udpTO);
	void proceedMDCX(MGCP &mgcp, const udp::endpoint& udpTO);
	void proceedRQNT(MGCP &mgcp, const udp::endpoint& udpTO);
	void proceedDLCX(MGCP &mgcp, const udp::endpoint& udpTO);

	int SetRoomID();
	SHP_CConfRoom CreateNewRoom();
	int GetFreePort();
	void SetFreePort(int port);
	SHP_CConfRoom FindRoom(string ID);
	int SDPFindMode(string SDP);
	SHP_Ann FindAnn(string ID);

private:
	std::vector<SHP_CConfRoom> RoomsVec_; // вектор существующих комнат
	std::vector<int> PortsinUse_; // вектор занятых портов
	std::vector<int> RoomsID_;
	std::vector<SHP_Ann> AnnVec_;
	std::vector<int>AnnID_;

	int countCRCX = 0;

	//CMGCPServer* server;
	
};

typedef std::shared_ptr<ConfControl> SHP_ConfControl;