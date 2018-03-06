#include "stdafx.h"
#include "MSsip_Cnf.h"
using namespace sip;


Cnf::Cnf(SHP_Caller point_)
{
	point_->SendToAnnModul("cr", "music_alaw.wav");
	vecPoints.push_back(point_);
	eventID = point_->GetParam("RoomID");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::AddPoint(SHP_Caller point_)
{
	vecPoints.push_back(point_);
	Process();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Cnf::RmPoint(SHP_Caller point_)
{
	vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
	Process();
	if (vecPoints.size() == 0) return true;
	else return false;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::Process()
{
	vector<SHP_Caller> active_points;
	for (auto& e : vecPoints) if (e->state == Caller::ready) active_points.push_back(e);

	if (state == false)
	{
		if (active_points.size() == 2)
		{
			SendCR(active_points);
			state = true;
		}
	}
	else
	{
		if (active_points.size() >= 2)
		{
			SendCR(active_points);
			state = true;
		}
		else
		{
			SendDL();
			active_points[0]->SendToAnnModul("cr", "music_alaw.wav");
			state = false;
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::SendDL()
{
	string result = "From=sip\n";
	result += "To=cnf\n";
	result += "EventID=sip" + eventID + "\n";
	result += "EventType=dl\n";
	COM::vecSigsIN[COM::INNER::cnf](result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::SendCR(vector<SHP_Caller> points_)
{
	string result = "From=sip\n";
	result += "To=cnf\n";
	result += "EventID=sip" + eventID + "\n";
	result += "EventType=cr\n";
	for (unsigned i = 0; i < points_.size(); ++i)
	{
		result += "CallID" + to_string(i) + "=" + points_[i]->GetParam("CallID") + "\n";
		if (points_[i]->playingAnn == true){ points_[i]->SendToAnnModul("dl", ""); }
	}
	COM::vecSigsIN[COM::INNER::cnf](result);
}
