#include "stdafx.h"
#include "LMGCP_LCnfRoom.h"
using namespace NLmgcpCnf;

Room::Room(string event_id_)
{
	mapData["EventID"] = event_id_;
	mapData["State"] = "Off";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Room::AddPoint(SHP_MGCP mgcp_)
{
	SHP_Point new_point; new_point.reset(new Point(mgcp_));
	if (mgcp_->Param("Error") == "")
	{
		mapPoint[mgcp_->Param("CallID")] = new_point;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Room::RemPoint(SHP_MGCP mgcp_)
{
	SHP_Point found_point = mapPoint[mgcp_->Param("CallID")];
	if (found_point == nullptr)
	{
		mgcp_->SetParam("Error", "Point not found");
		mapPoint.erase(mgcp_->Param("CallID"));
		return false;
	}
	mapPoint.erase(mgcp_->Param("CallID"));
	Proceed();
	return mapPoint.empty();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Room::ModPoint(SHP_MGCP mgcp_)
{
	SHP_Point found_point = mapPoint[mgcp_->Param("CallID")];
	if (found_point == nullptr)
	{
		mgcp_->SetParam("Error", "Point not found");
		mapPoint.erase(mgcp_->Param("CallID"));
		return;
	}
	found_point->ModPoint(mgcp_);
	Proceed();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Point Room::FindPoint(string call_id_)
{
	for (auto&e : mapPoint)
	{
		if (e.second->Param("CallID") == call_id_) return e.second;
	}
	return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Room::Proceed()
{
	vector<string> call_id;
	for (auto& e : mapPoint) if (e.second->Param("State") == "Active") call_id.push_back(e.second->Param("CallID"));
	if (Param("State") == "Off")
	{
		if (call_id.size() >= 3)
		{
			mapData["State"] = "On";
			SendModul("CR", call_id);
		}
		else {}
	}
	else
	{
		if (call_id.size() >= 2) SendModul("CR", call_id);
		else
		{
			mapData["State"] = "Off";
			SendModul("DL", vector<string>());
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Room::SendModul(string type_, vector<string> call_id_)
{
	string result = "IPL\n";
	result += "From=mgcp\n";
	result += "To=cnf\n";
	result += "CMD=" + type_ + "\n";
	result += "EventID=" + Param("EventID") + "\n";

	for (int i = 0; i < (int)call_id_.size(); ++i)
	{
		result += "CallID" + to_string(i) + "=" + call_id_[i] + "\n";
	}

	COM::SendModul("cnf", result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Room::Param(string name_)
{
	//3TODO
	if (name_ == "State") return mapData["State"];
	if (name_ == "EventID") return mapData["EventID"];
	if (name_ == "RoomSize") return to_string(mapPoint.size());
	else return "";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Room::PrintAll()
{
	//3TODO
	return "";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------