#include "stdafx.h"
#include "EventAnn.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventAnn::CRCX(SHP_MGCP mgcp_)
{
	BOOST_LOG_SEV(lg, trace) << "EventAnn::CRCX(...) for eventNum=" << mgcp_->data[MGCP::EventNum];
	string action_result = CheckExistance(mgcp_);
	if (action_result != "-1") // if ann found for some reason
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client already exists (0)."));
		return;
	}
	string server_port = ReservePort();
	mgcp_->data[MGCP::EventNum] = ReserveEventNum();
	string server_sdp = GenSDP(server_port, mgcp_);
	BOOST_LOG_SEV(lg, trace) << "Reserved:\n1) ServerPort=" << server_port << "\n2)EventNum=" << mgcp_->data[MGCP::EventNum] << "\n3)Server SDP=\n" << server_sdp;
	SHP_Ann new_ann = make_shared<Ann>(Ann(mgcp_, server_sdp, server_port));
	BOOST_LOG_SEV(lg, error) << "Created Ann with params:\n1)ID=" << new_ann->callID << "\n2)IP=" << new_ann->clientIP << "\n3)Port=" << new_ann->clientPort << "\n4)ServerPort=" << new_ann->serverPort;
	vecAnn.push_back(new_ann);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventAnn::RQNT(SHP_MGCP mgcp_)
{
	BOOST_LOG_SEV(lg, trace) << "EventAnn::RQNT(...) for eventNum=" << mgcp_->data[MGCP::EventNum];
	string action_result = CheckExistance(mgcp_);
	if (action_result == "-1" || action_result == "-2")
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found (1)."));
		return;
	}
	BOOST_LOG_SEV(lg, trace) << "EventAnn::RQNT(...): call to SHP_Ann found_ann = FindAnn(mgcp_);";
	SHP_Ann found_ann = FindAnn(mgcp_);
	if (found_ann == nullptr)
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found (2)."));
		return;
	}
	BOOST_LOG_SEV(lg, trace) << "EventAnn::RQNT(...): call to found_ann->RQNT(mgcp_);";
	found_ann->RQNT(mgcp_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventAnn::DLCX(SHP_MGCP mgcp_)
{
	BOOST_LOG_SEV(lg, trace) << "EventAnn::DLCX(...) for " << mgcp_->data[MGCP::EventNum];
	string action_result = CheckExistance(mgcp_);
	if (action_result == "-1" || action_result == "-2")
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found (4)."));
		return;
	}
	BOOST_LOG_SEV(lg, trace) << "EventAnn::DLCX(...): call to SHP_Ann found_ann = FindAnn(mgcp_);";
	SHP_Ann found_ann = FindAnn(mgcp_);
	if (found_ann == nullptr)
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found (5)."));
		return;
	}
	BOOST_LOG_SEV(lg, error) << "DELETED Ann with params:\n1)ID=" << found_ann->callID << "\n2)IP=" << found_ann->clientIP << "\n3)Port=" << found_ann->clientPort << "\n4)ServerPort=" << found_ann->serverPort;
	FreePort(found_ann->serverPort);
	BOOST_LOG_SEV(lg, trace) << "EventAnn::DLCX(...): FreePort(found_ann->serverPort); DONE";
	FreeEventNum(found_ann->eventNum);
	BOOST_LOG_SEV(lg, trace) << "EventAnn::DLCX(...): FreeEventNum(found_ann->eventNum); DONE";
	found_ann->DLCX(mgcp_);
	BOOST_LOG_SEV(lg, trace) << "EventAnn::DLCX(...): found_ann->DLCX(mgcp_); DONE";
	RemoveAnn(found_ann);
	BOOST_LOG_SEV(lg, trace) << "EventAnn::DLCX(...): RemoveAnn(found_ann); DONE";
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
SHP_Ann EventAnn::FindAnn(SHP_MGCP mgcp_)
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
void EventAnn::RemoveAnn(SHP_Ann ann_)
{
	vecAnn.erase(remove(vecAnn.begin(), vecAnn.end(), ann_), vecAnn.end());
}