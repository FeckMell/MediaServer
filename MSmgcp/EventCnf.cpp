#include "stdafx.h"
#include "EventCnf.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventCnf::CRCX(SHP_MGCP mgcp_)
{
	string action_result = CheckExistance(mgcp_);
	if (mgcp_->data[MGCP::EventNum] == "$")
	{
		if (action_result != "-1") // client with same callID already in cnf
		{
			ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client already exists."));
			return;
		}
		mgcp_->data[MGCP::EventNum] = ReserveEventNum();
		SHP_Cnf new_cnf = make_shared<Cnf>(Cnf(mgcp_));
		vecCnf.push_back(new_cnf);
	}

	if (action_result == "-2") // Error in data
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be created.4"));
		return;
	}

	SHP_Cnf cnf_found = FindCnf(mgcp_);
	if (cnf_found == nullptr)
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be created.5"));
		return;
	}
	string server_port = ReservePort();
	string server_sdp = GenSDP(server_port, mgcp_);

	cnf_found->CRCX(mgcp_, server_sdp, server_port);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventCnf::MDCX(SHP_MGCP mgcp_)
{
	string action_result = CheckExistance(mgcp_);
	if (action_result == "-1" || action_result == "-2")
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found."));
		return;
	}

	SHP_Cnf cnf_found = FindCnf(mgcp_);
	if (cnf_found == nullptr)
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found."));
		return;
	}
	cnf_found->MDCX(mgcp_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventCnf::DLCX(SHP_MGCP mgcp_)
{
	string action_result = CheckExistance(mgcp_);
	if (action_result == "-1") // not found 
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found.1"));
		return;
	}

	SHP_Cnf cnf_found = FindCnf(mgcp_);
	if (cnf_found == nullptr)
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found.2"));
		return;
	}
	action_result = cnf_found->DLCX(mgcp_);
	if (action_result != "")
	{
		FreePort(action_result);
		if (cnf_found->DeleteCnf())
		{
			//cout << "\nDEBUG delete cnf";
			RemoveCnf(cnf_found);
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string EventCnf::CheckExistance(SHP_MGCP mgcp_)
{
	string result;
	if (mgcp_->data[MGCP::EventNum] == "$")
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
			if (cnf->eventNum == mgcp_->data[MGCP::EventNum])
			{
				result = cnf->CheckExistance(mgcp_);
				if (result == "-1") return result;
				else return cnf->eventNum;
			}
		}
	}
	return "-1"; //return -1 - not found, -2 - error in data, else eventNum
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Cnf EventCnf::FindCnf(SHP_MGCP mgcp_)
{
	for (auto &cnf : vecCnf)
		if (cnf->eventNum == mgcp_->data[MGCP::EventNum]) return cnf;
	return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventCnf::RemoveCnf(SHP_Cnf cnf_)
{
	vecCnf.erase(std::remove(vecCnf.begin(), vecCnf.end(), cnf_), vecCnf.end());
	FreeEventNum(cnf_->eventNum);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------