#include "stdafx.h"
#include "EventCnf.h"

void EventCnf::CRCX(SHP_MGCP mgcp_)
{
	BOOST_LOG_SEV(lg, trace) << "EventCnf::CRCX(...) for " << mgcp_->data[MGCP::EventID];
	string action_result = CheckExistance(mgcp_);
	if (mgcp_->data[MGCP::EventID] == "$")
	{
		if (action_result != "-1") // client with same callID already in cnf
		{
			mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, "Error. Client already exists."));
			return;
		}
		mgcp_->data[MGCP::EventID] = ReserveEventID();
		BOOST_LOG_SEV(lg, debug) << "EventCnf::CRCX(...): reserved eventID=" << mgcp_->data[MGCP::EventID];
		SHP_Cnf new_cnf = make_shared<Cnf>(Cnf(mgcp_));
		BOOST_LOG_SEV(lg, trace) << "EventCnf::CRCX(...): SHP_Cnf new_cnf = make_shared<Cnf>(Cnf(mgcp_)); DONE";
		vecCnf.push_back(new_cnf);
		BOOST_LOG_SEV(lg, error) << "EventCnf::CRCX(...):Created Cnf with ID=" << new_cnf->eventID;
	}

	if (action_result == "-2") // Error in data
	{
		mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, "Error. Client could not be created.4"));
		return;
	}

	SHP_Cnf cnf_found = FindCnf(mgcp_);
	BOOST_LOG_SEV(lg, trace) << "EventCnf::CRCX(...): SHP_Cnf cnf_found = FindCnf(mgcp_); DONE";
	if (cnf_found == nullptr)
	{
		mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, "Error. Client could not be created.5"));
		return;
	}
	string server_port = ReservePort();
	string server_sdp = GenSDP(server_port, mgcp_);
	BOOST_LOG_SEV(lg, debug) << "EventCnf::CRCX(...): Reserved: 1)ServerPort=" << server_port << "\n2)ServerSDP=\n" << server_sdp;

	cnf_found->CRCX(mgcp_, server_sdp, server_port);
	BOOST_LOG_SEV(lg, debug) << "EventCnf::CRCX(...): cnf_found->CRCX(mgcp_, server_sdp, server_port); DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventCnf::MDCX(SHP_MGCP mgcp_)
{
	BOOST_LOG_SEV(lg, trace) << "EventCnf::MDCX(...) for " << mgcp_->data[MGCP::EventID];
	string action_result = CheckExistance(mgcp_);
	if (action_result == "-1" || action_result == "-2")
	{
		mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, "Error. Client could not be found."));
		return;
	}
	SHP_Cnf cnf_found = FindCnf(mgcp_);
	BOOST_LOG_SEV(lg, trace) << "EventCnf::MDCX(...): SHP_Cnf cnf_found = FindCnf(mgcp_); DONE";
	if (cnf_found == nullptr)
	{
		mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, "Error. Client could not be found."));
		return;
	}
	cnf_found->MDCX(mgcp_);
	BOOST_LOG_SEV(lg, trace) << "EventCnf::MDCX(...): cnf_found->MDCX(mgcp_); DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventCnf::DLCX(SHP_MGCP mgcp_)
{
	BOOST_LOG_SEV(lg, trace) << "EventCnf::DLCX(...) for " << mgcp_->data[MGCP::EventID];
	string action_result = CheckExistance(mgcp_);
	if (action_result == "-1") // not found 
	{
		mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, "Error. Client could not be found.1"));
		return;
	}

	SHP_Cnf cnf_found = FindCnf(mgcp_);
	BOOST_LOG_SEV(lg, trace) << "EventCnf::DLCX(...): SHP_Cnf cnf_found = FindCnf(mgcp_); DONE";
	if (cnf_found == nullptr)
	{
		mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, "Error. Client could not be found.2"));
		return;
	}
	action_result = cnf_found->DLCX(mgcp_);
	BOOST_LOG_SEV(lg, trace) << "EventCnf::DLCX(...): action_result = cnf_found->DLCX(mgcp_); DONE";
	if (action_result != "")
	{
		FreePort(action_result);
		BOOST_LOG_SEV(lg, debug) << "EventCnf::DLCX(...): freed port="<<action_result;
		if (cnf_found->DeleteCnf())
		{
			BOOST_LOG_SEV(lg, error) << "DELETED CNF with ID="<<cnf_found->eventID;
			RemoveCnf(cnf_found);
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string EventCnf::CheckExistance(SHP_MGCP mgcp_)
{
	string result;
	if (mgcp_->data[MGCP::EventID] == "$")
	{
		for (auto& cnf : vecCnf)
		{
			result = cnf->CheckExistance(mgcp_);
			if (result != "-1") return result;
		}
	}
	else
	{
		for (auto& cnf : vecCnf)
		{
			if (cnf->eventID == mgcp_->data[MGCP::EventID])
			{
				result = cnf->CheckExistance(mgcp_);
				if (result == "-1") return result;
				else return cnf->eventID;
			}
		}
	}
	return "-1"; //return -1 - not found, -2 - error in data, else eventID
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Cnf EventCnf::FindCnf(SHP_MGCP mgcp_)
{
	for (auto &cnf : vecCnf)
		if (cnf->eventID == mgcp_->data[MGCP::EventID]) return cnf;
	return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventCnf::RemoveCnf(SHP_Cnf cnf_)
{
	vecCnf.erase(std::remove(vecCnf.begin(), vecCnf.end(), cnf_), vecCnf.end());
	FreeEventID(cnf_->eventID);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------