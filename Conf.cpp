#pragma once
#include "stdafx.h"
#include "Conf.h"

//-*/-------------------------------------------------------------------------
void CConfRoom::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	steady_clock::time_point t1 = steady_clock::now();
	string result = DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	result += " ID=" + to_string(RoomID_) + " thread=" + boost::to_string(this_thread::get_id()) + "      ";
	CLogger.AddToLog(1, "\n" + result + a);
}
//-*/--------------------------Events-----------------------------------------
void CConfRoom::NewPoint(string SDPff, string SDPfc, string CallID, int port)
{
	loggit("void CConfRoom::NewInitPoint(string SDP)");
	SHP_CConfPoint point(new CConfPoint(SDPff, SDPfc, CallID, port, io_service_));
	//TODO:check for errors
	cllPoints_.push_back(point);
	loggit("void CConfRoom::NewInitPoint(string SDP) ENDED");
}
//-*/-------------------------------------------------------------------------
void CConfRoom::DeletePoint(string CallID)
{
	loggit("delete point for ID=" + CallID);
	SHP_CConfPoint Point;
	for (auto & entry : cllPoints_)
	{
		loggit("Compare Point ID=" + entry->CallID_);
		if (CallID == entry->CallID_)
		{
			loggit("Found Point");
			Point = entry;
			Point->mode = false;
			break;
		}
	}
	loggit("DeletePoint 1");
	Start();
	loggit("DeletePoint 2");
	Point->free();
	loggit("DeletePoint 3");
	cllPoints_.erase(std::remove(cllPoints_.begin(), cllPoints_.end(), Point), cllPoints_.end());
	loggit("Point erased");
}
//-*/-------------------------------------------------------------------------
string CConfRoom::ModifyPoint(SHP_CConfPoint Point, string SDPff)
{
	loggit("ModifyPoint");
	string result = Point->ModifyPoint(SDPff);
	Start();
	return result;
}
//-*/--------------------------Main function----------------------------------
void CConfRoom::Start()
{
	loggit("START");
	vector<SHP_CConfPoint> callers;
	for (auto& entry : cllPoints_)
		if (entry->mode == true)
			callers.push_back(entry);
	if ((on == false) && (callers.size() >= 3))
	{
		loggit("Mixer.reset");
		on = true;
		Mixer.reset(new CRTPReceive(callers, RoomID_));
	}
	else if ((on == true) && (callers.size() >= 2))
	{
		loggit("Mixer->add_track");
		Mixer->add_track(callers);
	}
	else if ((on == true) && (callers.size() == 1))
	{
		on = false;
		loggit("Mixer->Freeze(destroy)");
		//Mixer->Freeze();
		Mixer->destroy_all();
		Mixer.reset();
	}
	loggit("START END");
}
//-*/--------------------------Access to data---------------------------------
SHP_CConfPoint CConfRoom::FindPoint(string CallID)
{
	for (auto &e : cllPoints_)
	{
		if (e->CallID_ == CallID)
			return e;
	}
	return nullptr;
}
//-*/-------------------------------------------------------------------------
