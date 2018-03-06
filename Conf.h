#pragma once
#include "stdafx.h"
//#include "SrcCash.h"
#include "CRTPReceive.h"
#include "DestFusion.h"
extern FILE *FileLogConfPoint;
extern FILE *FileLogConfRoom;
/************************************************************************
	CConfPoint
************************************************************************/

class CConfPoint : public boost::noncopyable
{
public:
	CConfPoint(){ loggit("Point construct"); }

	void SetSDP(string SDP){ SDP_ = SDP; }
	string GetSDP(){ return SDP_; }

	void Set_output_addr(string addr){ output_addr = addr; }
	string Get_output_addr(){ return output_addr; }

	void SetID(string ID) { idPoint = ID; }
	string GetID() { return idPoint; }

	void SetPort(int port) { port_ = port; }
	int GetPort() { return port_; }


private:
	friend class CConfRoom;
	void loggit(string a);
	string SDP_;
	string output_addr;
	string idPoint; // CALLID поинта
	int port_; // приписанный этому поинту порт

	
};

typedef std::shared_ptr<CConfPoint> SHP_CConfPoint;


/************************************************************************
	CConfRoom
************************************************************************/
class CConfRoom : public boost::noncopyable
{
public:
	CConfRoom(){ loggit("Room construct"); }
	void NewInitPoint(string input_SDP, string output_SDP, string CallID, int port);
	void Start();
	void SetRoomID(int i) { RoomID_ = i; }
	int GetRoomID() { return RoomID_; }
	int GetNumCllPoints(){ return cllPoints_.size(); }
	string Make_addr_from_SDP(string output_SDP);
	SHP_CConfPoint FindPoint(string CallID);

private:
	void loggit(string a);
	SHP_CRTPReceive Mixer;
	std::vector<SHP_CConfPoint> cllPoints_;
	int RoomID_;
	
};
typedef std::shared_ptr<CConfRoom> SHP_CConfRoom;