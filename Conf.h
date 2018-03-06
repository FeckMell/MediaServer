#pragma once
#include "stdafx.h"
//#include "SrcCash.h"
#include "CRTPReceive.h"
#include "DestFusion.h"
#include "Structs.h"
//#include <boost/thread/thread.hpp>

struct NetworkData;

extern FILE *FileLogConfRoom;
/************************************************************************
	CConfPoint
************************************************************************/

struct CConfPoint
{
	void ModifySDP(int a);
	bool mode = true; // active, inactive (hold)
	void loggit(string a);
	string SDP_; // sdp ��� ffmpeg
	string SDP_for_client; 
	string CallID_; // CALLID ������
	int my_port_; // ����������� ����� ������ ����
	int remote_port_; // ���� �������
	string remote_ip_;// ip �������
};

typedef std::shared_ptr<CConfPoint> SHP_CConfPoint;


/************************************************************************
	CConfRoom
************************************************************************/
class CConfRoom
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
	string GetPointID(int i){ return cllPoints_[i]->SDP_; }

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