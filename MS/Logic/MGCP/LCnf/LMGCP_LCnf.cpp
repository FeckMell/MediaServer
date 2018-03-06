#include "stdafx.h"
#include "LMGCP_LCnf.h"
using namespace NLmgcpCnf;
map<string, SHP_Room> LCnf::mapRoom = {};
vector<int> LCnf::vecEventID = {};

void LCnf::Init(map<string, boost::function<void(boost::any)>>& map_func_)
{
	LOG::AddLogInstance("MGCPLCnf");
	map_func_["cnf"] = boost::bind(&LCnf::Proceed, _1);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LCnf::Proceed(boost::any mess_)
{
	try
	{
		SHP_IPL ipl = boost::any_cast<SHP_IPL>(mess_);
		ProceedIPL(ipl);
	}
	catch (...)
	{
		SHP_MGCP mgcp = boost::any_cast<SHP_MGCP>(mess_);
		ProceedMGCP(mgcp);
		return;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LCnf::ProceedMGCP(SHP_MGCP mgcp_)
{
	if (mgcp_->Param("CMD") == "CRCX")
	{
		CRCX(mgcp_);
	}
	else if (mgcp_->Param("CMD") == "MDCX")
	{
		MDCX(mgcp_);
	}
	else if (mgcp_->Param("CMD") == "DLCX")
	{
		DLCX(mgcp_);
	}
	else
	{
		//2TODO
		LOG::Log("fatal", "Errors", "NLmgcpCnf::LCnf::ProceedMGCP CMD:" + mgcp_->Param("CMD"));
		//system("pause");
		//exit(-1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LCnf::ProceedIPL(SHP_IPL ipl_)
{
	if (ipl_->Param("CMD") == "DL")
	{
		for (auto& e : mapRoom)
		{
			SHP_Point found_point = e.second->FindPoint(ipl_->Param("CallID"));
			if (found_point != nullptr)
			{
				SHP_MGCP mgcp; mgcp.reset(new MGCP());
				mgcp->SetParam("CallID", ipl_->Param("CallID"));
				bool del_room = e.second->RemPoint(mgcp);
				if (del_room)
				{
					string event_id = e.second->Param("EventID");
					FreeEventID(event_id);
					mapRoom.erase(event_id);
				}
				return;
			}
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LCnf::CRCX(SHP_MGCP mgcp_)
{
	if (mgcp_->Param("EventID") == "$")
	{
		string event_id = ReserveEventID();
		mgcp_->SetParam("EventID", event_id);
		SHP_Room new_room; new_room.reset(new Room(event_id));
		mapRoom[event_id] = new_room;

	}
	SHP_Room found_room = mapRoom[mgcp_->Param("EventID")];
	if (found_room == nullptr)
	{
		mgcp_->SetParam("Error", "Not found room");
		return;
	}
	found_room->AddPoint(mgcp_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LCnf::MDCX(SHP_MGCP mgcp_)
{
	SHP_Room found_room = mapRoom[mgcp_->Param("EventID")];
	if (found_room == nullptr)
	{
		mgcp_->SetParam("Error", "Not found");
		return;
	}
	found_room->ModPoint(mgcp_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LCnf::DLCX(SHP_MGCP mgcp_)
{
	SHP_Room found_room = mapRoom[mgcp_->Param("EventID")];
	if (found_room == nullptr)
	{
		mgcp_->SetParam("Error", "Not found");
		return;
	}
	bool del_room = found_room->RemPoint(mgcp_);
	if (del_room)
	{
		string event_id = found_room->Param("EventID");
		FreeEventID(event_id);
		mapRoom.erase(event_id);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string LCnf::ReserveEventID()
{
	int free_event_id = 0;
	if (vecEventID.size() == 0) { vecEventID.push_back(free_event_id);  return to_string(free_event_id); }
	for (unsigned i = 0; i <vecEventID.size(); ++i)
	{
		if (vecEventID[i] != free_event_id)
		{
			vecEventID.push_back(free_event_id);
			sort(vecEventID.begin(), vecEventID.end());
			return to_string(free_event_id);
		}
		free_event_id++;
	}
	vecEventID.push_back(free_event_id);
	sort(vecEventID.begin(), vecEventID.end());
	return to_string(free_event_id);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LCnf::FreeEventID(string event_id_)
{
	vecEventID.erase(remove(vecEventID.begin(), vecEventID.end(), stoi(event_id_)), vecEventID.end());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------