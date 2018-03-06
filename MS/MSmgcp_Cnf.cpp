#include "stdafx.h"
#include "MSmgcp_Cnf.h"
using namespace mgcp;


Cnf::Cnf(SHP_Point point_, string event_id_, SHP_SDP c_SDP_) : eventID(event_id_)
{
	vecPoints.push_back(point_);
	point_->ModifyClientSDP(c_SDP_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::AddPoint(SHP_Point point_)
{
	vecPoints.push_back(point_);
	//state = false;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Cnf::DeletePoint(SHP_Point point_)
{
	vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());

	if (vecPoints.size() == 0){ return true; }
	else { return false; }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::Process()
{
	vector<SHP_Point> active_points;
	for (auto& e : vecPoints) { 
		if (e->GetParam("State") == "ready") active_points.push_back(e);
		cout << "\nPoint:" + e->GetParam("CallID") + "_ state=" + e->GetParam("State") + "_";
	}

	if (state == false)
	{
		if (active_points.size() == 3)
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
			state = false;
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::SendDL()
{
	string result = "From=mgcp\n";
	result += "To=cnf\n";
	result += "EventID=mgcp" + eventID + "\n";
	result += "EventType=dl\n";
	COM::vecSigsIN[COM::INNER::cnf](result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::SendCR(vector<SHP_Point> points_)
{
	string result = "From=mgcp\n";
	result += "To=cnf\n";
	result += "EventID=mgcp" + eventID + "\n";
	result += "EventType=cr\n";
	for (unsigned i = 0; i < points_.size(); ++i)
	{
		result += "CallID" + to_string(i) + "=" + points_[i]->GetParam("CallID") + "\n";
	}
	COM::vecSigsIN[COM::INNER::cnf](result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------