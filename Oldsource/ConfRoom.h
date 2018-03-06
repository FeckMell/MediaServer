#pragma once
#ifdef WIN32
#include "stdafx.h"
#endif
#ifdef linux
#include "stdinclude.h"
#endif
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
	CConfRoom(){ on = false; loggit("Room construct"); }

	void NewPoint(string SDPff, string SDPfc, string CallID, int port);
	void DeletePoint(string CallID);
	string ModifyPoint(SHP_CConfPoint Point, string SDPff);

	void SetRoomID(int i) { RoomID_ = i; }
	int GetRoomID() { return RoomID_; }
	int GetNumCllPoints(){ return cllPoints_.size(); }
	
	SHP_CConfPoint FindPoint(string CallID);

	void testdelete(){ Mixer.reset(); Mixer.~shared_ptr(); cout << "\ntestdelete"; }

private:
	void Start();
	void loggit(string a);
	
	//SHP_Ann OAnn;
	SHP_ConfAudio Mixer; // destructor problem here!!
	std::vector<SHP_CConfPoint> cllPoints_;
	int RoomID_;
	bool on;
	boost::asio::io_service io_service_;
};
typedef std::shared_ptr<CConfRoom> SHP_CConfRoom;