#include "stdafx.h"
#include "Conf.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Conf::Conf(SHP_MGCP mgcp_)
{
	eventNum = mgcp_->data[MGCP::EventNum];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Conf::CRCX(SHP_MGCP mgcp_, string server_sdp_, string server_port_)
{
	SHP_CallerBase new_point = make_shared<CallerBase>(CallerBase(mgcp_, server_sdp_, server_port_));
	vecCallerBase.push_back(new_point);
	ReplyClient(mgcp_, mgcp_->ResponseOK(200, "add event type") + "\n\n" + server_sdp_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Conf::MDCX(SHP_MGCP mgcp_)
{
	SHP_CallerBase found_point = FindCallerBase(mgcp_);
	if (found_point==nullptr)
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found.3"));
		return;
	}
	string action_result = found_point->ModifyCallerBase(mgcp_);
	if (action_result == "0") // client sent first SDP
	{
		ReplyClient(mgcp_, mgcp_->ResponseOK(200, ""));
		return;
	}
	else if (action_result == "1") //client changed SDP
	{
		ReplyClient(mgcp_, mgcp_->ResponseOK(200, "") + "\n\n" + found_point->server_SDP);
		return;
	}
	else // some error
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, action_result));
		return;
	}

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Conf::DLCX(SHP_MGCP mgcp_)
{
	SHP_CallerBase found_point = FindCallerBase(mgcp_);
	if (found_point == nullptr)
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found."));
		return "";
	}
	string action_result = found_point->server_Port;
	RemoveCallerBase(found_point);
	ReplyClient(mgcp_, mgcp_->ResponseOK(250, ""));
	return action_result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Conf::CheckExistance(SHP_MGCP mgcp_)
{
	string result;
	for (auto& point : vecCallerBase)
	{
		if (point->callID == mgcp_->data[MGCP::ParamC])
			return point->callID;
	}
	return "-1"; //return -1 - not found, -2 - error in data, else eventNum
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_CallerBase Conf::FindCallerBase(SHP_MGCP mgcp_)
{
	for (auto &point : vecCallerBase)
	if (point->callID == mgcp_->data[MGCP::ParamC]) return point;
	return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Conf::RemoveCallerBase(SHP_CallerBase point_)
{
	vecCallerBase.erase(std::remove(vecCallerBase.begin(), vecCallerBase.end(), point_), vecCallerBase.end());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------