#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "Functions.h"
#include "CRTPReceive.h"
#include "Structs.h"
//#include <boost/thread/thread.hpp>

struct NetworkData;
extern string DateStr;
/************************************************************************
	CConfPoint
************************************************************************/

struct CConfPoint
{
	void ModifySDP(int a);
	void ChangeMode(string SDP);
	string ChangeVersion(string SDP);
	bool mode = false; // active, inactive (hold)
	void loggit(string a);


	string SDP_; // sdp для ffmpeg
	string SDP_for_client; 
	string CallID_; // CALLID поинта
	int my_port_; // приписанный этому поинту порт
	int remote_port_; // порт клиента
	string remote_ip_;// ip клиента
	bool record = false;
};

typedef std::shared_ptr<CConfPoint> SHP_CConfPoint;


/************************************************************************
	CConfRoom
************************************************************************/
class CConfRoom
{
public:
	CConfRoom(){ on = false; loggit("Room construct"); }

	void DeletePoint(string CallID);
	void NewInitPoint(string SDPff, string SDPfc, string CallID, int port);

	void Start();
	void SetRoomID(int i) { RoomID_ = i; }
	int GetRoomID() { return RoomID_; }
	int GetNumCllPoints(){ return cllPoints_.size(); }
	
	SHP_CConfPoint FindPoint(string CallID);
	string GetPointID(int i){ return cllPoints_[i]->SDP_; }
	string ModifyPoint(SHP_CConfPoint Point, string SDPff); 

private:
	void loggit(string a);
	NetworkData FillNetData();
	SHP_CRTPReceive Mixer;
	string Make_addr_from_SDP(string output_SDP);

	std::vector<SHP_CConfPoint> cllPoints_;
	int RoomID_;
	bool on;
	int counter = 0;
	
};
typedef std::shared_ptr<CConfRoom> SHP_CConfRoom;