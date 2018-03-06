#include "stdafx.h"
#include "LogicAnn.h"

LogicAnn::LogicAnn()
{

}
//*///------------------------------------------------------------------------------------------
map<string, boost::any> LogicAnn::RegisterMePlease()
{
	map<string, boost::any> result;
	result.insert({ "MGCP", boost::bind(&LogicAnn::ProceedMGCP, this, _1) });
	result.insert({ "IPL", boost::bind(&LogicAnn::ProceedIPL, this, _1) });
	result.insert({ "CTRL", boost::bind(&LogicAnn::ProceedCTRL, this, _1) });
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicAnn::ProceedMGCP(SHP_MGCP mgcp_)
{
	if (mgcp_->Param("CMD") == "CRCX") MGCP_CRCX(mgcp_);
	else if (mgcp_->Param("CMD") == "RQNT") MGCP_RQNT(mgcp_);
	else if (mgcp_->Param("CMD") == "DLCX") MGCP_DLCX(mgcp_);
	else 
	{
		LOG::Log("ERRORS", "fatal", "Unsupported request:" + mgcp_->Param("Request"));
	}
	mgcp_->Reply();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicAnn::ProceedIPL(/*params*/){}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicAnn::ProceedCTRL(/*params*/){}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicAnn::MGCP_CRCX(SHP_MGCP mgcp_)
{
	if (CALLERSTORE::CheckExistance(mgcp_->Param("CallID"), mgcp_->Param("ClientIP"), mgcp_->Param("ClientPort")))
	{
		LOG::Log("ERRORS", "fatal", "Point Already Exists:" + mgcp_->Param("Request"));
		mgcp_->SetParam("Error", "Point Already Exists");
		/* Do something*/
		return; //temp, until do something 
	}
	mgcp_->SetSDP("ServerSDP", make_shared<SDP>(SOCKETSTORE::ReservePort(), mgcp_->Param("CallID")));
	_mapannpoints[mgcp_->Param("EventID")] = make_shared<LogicAnnPoint>(mgcp_->GetSDP("ClientSDP"), mgcp_->GetSDP("ServerSDP"));
	mgcp_->SetParam("Error", _mapannpoints[mgcp_->Param("EventID")]->Param("Error"));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicAnn::MGCP_RQNT(SHP_MGCP mgcp_)
{
	if (_mapannpoints[mgcp_->Param("EventID")] == nullptr)
	{
		LOG::Log("ERRORS", "fatal", "Point Does Not Exist:" + mgcp_->Param("Request"));
		_mapannpoints.erase(mgcp_->Param("EventID"));
		mgcp_->SetParam("Error", "Point not found");
		return;
	}
	_mapannpoints[mgcp_->Param("EventID")]->RequestMusic(mgcp_->Param("S"), mgcp_->Param("Q"));
	mgcp_->SetParam("Error", _mapannpoints[mgcp_->Param("EventID")]->Param("Error"));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicAnn::MGCP_DLCX(SHP_MGCP mgcp_)
{
	if (_mapannpoints[mgcp_->Param("EventID")] == nullptr)
	{
		LOG::Log("ERRORS", "fatal", "Point Does Not Exist:" + mgcp_->Param("Request"));
		_mapannpoints.erase(mgcp_->Param("EventID"));
		mgcp_->SetParam("Error", "Point not found");
		return;
	}
	_mapannpoints[mgcp_->Param("EventID")]->StopMusic();
	mgcp_->SetParam("Error", _mapannpoints[mgcp_->Param("EventID")]->Param("Error"));
	_mapannpoints.erase(mgcp_->Param("EventID"));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicAnn::MGCP_NTFY(/**/){}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------