#include "stdafx.h"
#include "EventCnf.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventCnf::CRCX(SHP_MGCP mgcp_)
{
	string action_result = CheckExistance(mgcp_);
	if (mgcp_->data[MGCP::EventNum] == "$")
	{
		if (action_result != "-1") // client with same callID already in conf
		{
			ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client already exists."));
			return;
		}
		mgcp_->data[MGCP::EventNum] = ReserveEventNum();
		SHP_Conf new_conf = make_shared<Conf>(Conf(mgcp_));
		vecConf.push_back(new_conf);
	}

	if (action_result == "-2") // Error in data
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be created.4"));
		return;
	}

	SHP_Conf conf_found = FindConf(mgcp_);
	if (conf_found == nullptr)
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be created.5"));
		return;
	}
	string server_port = ReservePort();
	string server_sdp = GenSDP(server_port, mgcp_);

	conf_found->CRCX(mgcp_, server_sdp, server_port);
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

	SHP_Conf conf_found = FindConf(mgcp_);
	if (conf_found == nullptr)
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found."));
		return;
	}
	conf_found->MDCX(mgcp_);
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

	SHP_Conf conf_found = FindConf(mgcp_);
	if (conf_found == nullptr)
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found.2"));
		return;
	}
	action_result = conf_found->DLCX(mgcp_);
	if (action_result != "")
	{
		FreePort(action_result);
		if (conf_found->DeleteConf())
		{
			//cout << "\nDEBUG delete conf";
			RemoveConf(conf_found);
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
		for (auto& conf : vecConf)
		{
			result = conf->CheckExistance(mgcp_);
			if (result != "-1") return result;
		}
	}
	else
	{
		for (auto& conf : vecConf)
		{
			if (conf->eventNum == mgcp_->data[MGCP::EventNum])
			{
				result = conf->CheckExistance(mgcp_);
				if (result == "-1") return result;
				else return conf->eventNum;
			}
		}
	}
	return "-1"; //return -1 - not found, -2 - error in data, else eventNum
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Conf EventCnf::FindConf(SHP_MGCP mgcp_)
{
	for (auto &conf : vecConf)
		if (conf->eventNum == mgcp_->data[MGCP::EventNum]) return conf;
	return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void EventCnf::RemoveConf(SHP_Conf conf_)
{
	vecConf.erase(std::remove(vecConf.begin(), vecConf.end(), conf_), vecConf.end());
	FreeEventNum(conf_->eventNum);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------