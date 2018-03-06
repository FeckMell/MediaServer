#pragma once
#include "stdafx.h"
//#include "SrcCash.h"
#include "CRTPReceive.h"
#include "DestFusion.h"
#include <boost/thread/thread.hpp>
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

	//void Set_output_addr(string addr){ output_addr = addr; }
	//string Get_output_addr(){ return output_addr; }

	void SetID(string ID) { idPoint = ID; }
	string GetID() { return idPoint; }

	void SetMyPort(int port) { my_port_ = port; }
	int GetMyPort() { return my_port_; }

	void SetRemotePort(int port) { remote_port_ = port; }
	int GetRemotePort() { return remote_port_; }

	void SetRemoteIP(string IP) { remote_ip_ = IP; }
	string GetRemoteIP() { return remote_ip_; }


	
	void ModifySDP(int a);
	bool mode = true;
private:
	friend class CConfRoom;
	void loggit(string a);
	string SDP_;
	string idPoint; // CALLID поинта
	int my_port_; // приписанный этому поинту порт
	int remote_port_; // порт клиента
	string remote_ip_;// ip клиента
	

	
};

typedef std::shared_ptr<CConfPoint> SHP_CConfPoint;


/************************************************************************
	CConfRoom
************************************************************************/
class CConfRoom : public boost::noncopyable
{
public:
	CConfRoom(){ on = false; loggit("Room construct"); }
	std::vector<SHP_CConfPoint> GetAllPoints(){ return cllPoints_; }
	int DeletePoint(string CallID);
	void NewInitPoint(string SDP, string CallID, int port);
	void Start();
	void SetRoomID(int i) { RoomID_ = i; }
	int GetRoomID() { return RoomID_; }
	int GetNumCllPoints(){ return cllPoints_.size(); }
	string Make_addr_from_SDP(string output_SDP);
	SHP_CConfPoint FindPoint(string CallID);
	string GetPointID(int i){ return cllPoints_[i]->GetID(); }

	string MakeRemotePort(string SDP);
	string MakeRemoteIP(string SDP);
private:
	void loggit(string a);
	SHP_CRTPReceive Mixer;
	std::vector<SHP_CConfPoint> cllPoints_;
	int RoomID_;
	bool on;
	int counter = 0;
	boost::shared_ptr<boost::thread> thread;
	//boost::shared_ptr<boost::thread> thread(new boost::thread thread);
	
};
typedef std::shared_ptr<CConfRoom> SHP_CConfRoom;