#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "Functions.h"
#include "ConfAudio.h"
#include "Structs.h"
#include "ConfPoint.h"
#include "Ann.h"

//#include <boost/thread/thread.hpp>

extern Logger* CLogger;
extern string DateStr;
/************************************************************************
	CConfRoom
************************************************************************/
class CConfRoom
{
public:
	enum State { OFF, ON, PAUSED, DESTROY };
	CConfRoom(){ loggit("Room construct"); }
	~CConfRoom(){ Mixer.reset(); Mixer.~shared_ptr(); }

	void NewPoint(string SDPff, string SDPfc, string CallID, int port);
	void DeletePoint(string CallID);
	string ModifyPoint(SHP_CConfPoint Point, string SDPff);

	void SetRoomID(int i) { RoomID_ = i; }
	int GetRoomID() { return RoomID_; }
	int GetNumCllPoints(){ return cllPoints_.size(); }
	
	SHP_CConfPoint FindPoint(string CallID);

private:
	void Start();
	void loggit(string a);
	
	State state_ = OFF;
	SHP_ConfAudio Mixer;
	std::vector<SHP_CConfPoint> cllPoints_;
	int RoomID_;
	boost::asio::io_service io_service_;
};
typedef std::shared_ptr<CConfRoom> SHP_CConfRoom;