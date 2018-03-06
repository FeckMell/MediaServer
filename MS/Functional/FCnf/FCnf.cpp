#include "stdafx.h"
#include "FCnf.h"
using namespace NFCnf;
map<string, SHP_Room> FCnf::mapRoom = {};

void FCnf::Init()
{
	LOG::AddLogInstance("FCnf");
	COM::RegModul("cnf", boost::function<void(string)>(&FCnf::ProceedRequest, _1));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void FCnf::ProceedRequest(string request_)
{
	SHP_IPL ipl; ipl.reset(new IPL(request_));
	if (ipl->Param("CMD") == "CR") Create(ipl);
	else if (ipl->Param("CMD") == "DL") Delete(ipl);
	else
	{
		LOG::Log("fatal", "Errors", "FCnf::ProceedRequest undone CMD:\n" + request_);
		cout << "\nFAnn::ProceedRequest undone:\n" << request_;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void FCnf::Create(SHP_IPL ipl_)
{
	SHP_Room new_room = mapRoom[ipl_->Param("EventID")];
	if (new_room != nullptr)
	{
		mapRoom.erase(ipl_->Param("EventID"));
		new_room.reset();
	}
	new_room.reset(new Room(ipl_));
	mapRoom[ipl_->Param("EventID")] = new_room;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void FCnf::Delete(SHP_IPL ipl_)
{
	mapRoom.erase(ipl_->Param("EventID"));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------