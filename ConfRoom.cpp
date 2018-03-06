#include "stdafx.h"
#include "ConfRoom.h"

//*///------------------------------------------------------------------------------------------
void CConfRoom::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	steady_clock::time_point t1 = steady_clock::now();
	string result = DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	result += " ID=" + to_string(RoomID_) + " thread=" + boost::to_string(this_thread::get_id()) + "      ";
	CLogger->AddToLog(1, "\n" + result + a);
}
//-*/--------------------------Events-----------------------------------------
void CConfRoom::NewPoint(string SDPff, string SDPfc, string CallID, int port)
{
	loggit("void CConfRoom::NewInitPoint");
	SHP_CConfPoint point(new CConfPoint(SDPff, SDPfc, CallID, port, io_service_));
	//TODO:check for errors
	cllPoints_.push_back(point);
	loggit("void CConfRoom::NewInitPoint ENDED");
}
//*///------------------------------------------------------------------------------------------
void CConfRoom::DeletePoint(string CallID)
{
	loggit("delete point for ID=" + CallID);
	SHP_CConfPoint Point;
	for (auto & entry : cllPoints_)
	{
		if (CallID == entry->CallID_)
		{
			loggit("Found Point");
			Point = entry;
			Point->mode = false;
			Start();
			break;
		}
	}
	
	cllPoints_.erase(std::remove(cllPoints_.begin(), cllPoints_.end(), Point), cllPoints_.end());
	Point->free();
	loggit("Point " + CallID + " erased");
}
//*///------------------------------------------------------------------------------------------
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
	if (state_ == OFF)
	{
		if (callers.size() >= 3)
		{
			loggit("Starting conference.");
			state_ = ON;
			Mixer.reset(new ConfAudio(callers, RoomID_));
		}
		else
		{
			loggit("Room state was OFF and callers size=" + to_string(callers.size()) + ", so do nothing.");
		}
	}
	else if (state_ == ON)
	{
		if (callers.size() >= 2)
		{
			loggit("Adding/removing caller from conference. Now " + to_string(callers.size()) + ".");
			Mixer->add_track(callers);
		}
		else if (callers.size() == 1)
		{
			loggit("Conference PAUSED.");
			Mixer->destroy_all();
			state_ = PAUSED;
		}
		else
		{
			loggit("Someting wrong: state=ON, callers size !>=2 or !=1. ERROR.");
		}
	}
	else if (state_ == PAUSED)
	{
		if (callers.size() == 0)
		{
			loggit("Conference is going to be DESTROYED.");
			state_ = DESTROY;
			Mixer.reset();
		}
		else if (callers.size() >= 2)
		{
			loggit("Conference change state from PAUSED to ON");
			state_ = ON;
			Mixer.reset(new ConfAudio(callers, RoomID_));
		}
		else
		{
			loggit("Someting wrong: state=PAUSED, callers size !>=2 or !=0. ERROR.");
		}
	}
	else if (state_ == DESTROY)
	{
		loggit("Someting wrong: state=DESTROY, but conference was called. ERROR.");
	}
	else
	{
		loggit("Someting wrong: call to unknown state. ERROR.");
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
//*///------------------------------------------------------------------------------------------
