#include "stdafx.h"
#include "LMGCP_LCnfPoint.h"
using namespace NLmgcpCnf;

Point::Point(SHP_MGCP mgcp_)
{
	mapData["State"] = "Inactive";
	CallerStore::CreateCaller(mgcp_->Param("CallID"));
	baseCaller = CallerStore::GetCaller(mgcp_->Param("CallID"));
	baseCaller->SetParam("Creator", "mgcp");
	baseCaller->SetParam("Type", "cnf");
	mgcp_->SetSDP("Server", baseCaller->GetSDP("Server"));
	if (mgcp_->GetSDP("Client") != nullptr)
	{
		baseCaller->SetSDP("Client", mgcp_->GetSDP("Client"));
		if (baseCaller->GetSDP("Client")->Param("Mode") == "sendrecv") mapData["State"] = "Active";
	}
}
Point::~Point()
{
	CallerStore::DelCaller(Param("CallID"));
}
void Point::ModPoint(SHP_MGCP mgcp_)
{
	if (baseCaller->GetSDP("Client") == nullptr) 
	{ 
		baseCaller->SetSDP("Client", mgcp_->GetSDP("Client"));
		mapData["State"] = "Active";
	}
	else
	{
		baseCaller->SetSDP("Client", mgcp_->GetSDP("Client"));
		SHP_SDP server_sdp = baseCaller->GetSDP("Server");
		server_sdp->ChangeModeS(baseCaller->GetSDP("Client")->Param("Mode"));
		mgcp_->SetSDP("Server", server_sdp);
		if (server_sdp->Param("Mode") == "sendrecv") mapData["State"] = "Active";
		else mapData["State"] = "Inactive";
	}
}
string Point::Param(string name_)
{
	if (name_ == "State") return mapData["State"];
	else return baseCaller->Param(name_);
}
string Point::PrintAll()
{
	//3TODO
	return "";
}