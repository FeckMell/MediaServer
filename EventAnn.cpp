#include "stdafx.h"
#include "EventAnn.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventAnn::CRCX(SHP_MGCP mgcp_)
{
	string action_result = CheckExistance(mgcp_);
	if (action_result != "-1") // if ann found for some reason
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client already exists (0)."));
		return;
	}
	string server_port = ReservePort();
	mgcp_->data[MGCP::EventNum] = ReserveEventNum();
	string server_sdp = GenSDP(server_port, mgcp_);
	SHP_CallerAnn new_ann = make_shared<CallerAnn>(CallerAnn(mgcp_, server_sdp, server_port));
	vecAnn.push_back(new_ann);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventAnn::RQNT(SHP_MGCP mgcp_)
{
	string action_result = CheckExistance(mgcp_);
	if (action_result == "-1" || action_result == "-2")
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found (1)."));
		return;
	}

	SHP_CallerAnn found_ann = FindAnn(mgcp_);
	if (found_ann == nullptr)
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found (2)."));
		return;
	}

	found_ann->RQNT(mgcp_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventAnn::DLCX(SHP_MGCP mgcp_)
{
	string action_result = CheckExistance(mgcp_);
	//cout << "\nDEBUG ann action_result=" << action_result;
	if (action_result == "-1" || action_result == "-2")
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found (4)."));
		return;
	}

	SHP_CallerAnn found_ann = FindAnn(mgcp_);
	if (found_ann == nullptr)
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found (5)."));
		return;
	}

	FreePort(found_ann->server_Port);
	FreeEventNum(found_ann->eventNum);
	found_ann->DLCX(mgcp_);
	RemoveAnn(found_ann);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string EventAnn::CheckExistance(SHP_MGCP mgcp_)
{
	if (mgcp_->data[MGCP::EventNum] == "$")
	{
		for (auto& ann : vecAnn)
		{
			if (ann->callID == mgcp_->data[MGCP::ParamC])
				return ann->eventNum;
		}
	}
	else
	{
		for (auto& ann : vecAnn)
		{
			if (ann->eventNum == mgcp_->data[MGCP::EventNum])
			{
				if (ann->callID == mgcp_->data[MGCP::ParamC])
					return ann->eventNum;
				else return "-2";
			}
		}
	}
	return "-1"; //return -1 - not found, -2 - error in data, else eventNum
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_CallerAnn EventAnn::FindAnn(SHP_MGCP mgcp_)
{
	for (auto &ann : vecAnn)
	{
		if (ann->eventNum == mgcp_->data[MGCP::EventNum])
		{
			if (ann->callID == mgcp_->data[MGCP::ParamC])
				return ann;
		}
	}
	return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventAnn::RemoveAnn(SHP_CallerAnn ann_)
{
	vecAnn.erase(remove(vecAnn.begin(), vecAnn.end(), ann_), vecAnn.end());
}