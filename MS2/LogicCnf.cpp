#include "stdafx.h"
#include "LogicCnf.h"

LogicCnf::LogicCnf()
{

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
map<string, boost::any> LogicCnf::RegisterMePlease()
{
	map<string, boost::any> result;
	result.insert({ "MGCP", boost::bind(&LogicCnf::ProceedMGCP, this, _1) });
	result.insert({ "IPL", boost::bind(&LogicCnf::ProceedIPL, this, _1) });
	result.insert({ "CTRL", boost::bind(&LogicCnf::ProceedCTRL, this, _1) });
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicCnf::ProceedMGCP(SHP_MGCP mgcp_)
{
	if (mgcp_->Param("CMD") == "CRCX") MGCP_CRCX(mgcp_);
	else if (mgcp_->Param("CMD") == "MDCX") MGCP_MDCX(mgcp_);
	else if (mgcp_->Param("CMD") == "DLCX") MGCP_DLCX(mgcp_);
	else
	{
		LOG::Log("ERRORS", "fatal", "Unsupported request:" + mgcp_->Param("Request"));
	}
	mgcp_->Reply();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicCnf::ProceedIPL(/*params*/){}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicCnf::ProceedCTRL(/*params*/){}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicCnf::MGCP_CRCX(SHP_MGCP mgcp_)
{
	if (CALLERSTORE::CheckExistance(mgcp_->Param("CallID"), mgcp_->Param("ClientIP"), mgcp_->Param("ClientPort")))
	{
		LOG::Log("ERRORS", "fatal", "Point Already Exists:" + mgcp_->Param("Request"));
		mgcp_->SetParam("Error", "Point Already Exists");
		/* Do something*/
		return; //temp, until do something 
	}
	//mgcp_->SetSDP("ServerSDP", make_shared<SDP>(SOCKETSTORE::ReservePort(), mgcp_->Param("CallID")));
	//_mapannpoints[mgcp_->Param("EventID")] = make_shared<LogicAnnPoint>(mgcp_->GetSDP("ClientSDP"), mgcp_->GetSDP("ServerSDP"));
	//mgcp_->SetParam("Error", _mapannpoints[mgcp_->Param("EventID")]->Param("Error"));
	mgcp_->SetSDP("ServerSDP", make_shared<SDP>(SOCKETSTORE::ReservePort(), mgcp_->Param("CallID")));
	_mapcnfpoints[mgcp_->Param("CallID")] = make_shared<LogicCnfPoint>(mgcp_->GetSDP("ClientSDP"), mgcp_->GetSDP("ServerSDP"));
	_mapcnfrooms

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicCnf::MGCP_MDCX(SHP_MGCP mgcp_){}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicCnf::MGCP_DLCX(SHP_MGCP mgcp_){}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------