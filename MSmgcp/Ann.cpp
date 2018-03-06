#include "stdafx.h"
#include "Ann.h"

Ann::Ann(SHP_MGCP mgcp_, string server_sdp_, string server_port_)
:CallerBase(mgcp_, server_sdp_, server_port_)
{
	BOOST_LOG_SEV(lg, trace) << "Ann::Ann(...)";
	eventID = mgcp_->data[MGCP::EventID];
	BOOST_LOG_SEV(lg, trace) << "Set this Ann eventID=" << eventID;
	//ReplyClient(mgcp_, mgcp_->ResponseOK(200,"add event type") + "\n\n" + server_sdp_);
	mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseOK(200, "add event type") + "\n\n" + server_sdp_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::RQNT(SHP_MGCP mgcp_)
{
	BOOST_LOG_SEV(lg, trace) << "Ann::RQNT(...) for Ann " << eventID;
	fileName = get_substr(mgcp_->data[MGCP::ParamS], ",file:///", ")");
	BOOST_LOG_SEV(lg, info) << "Ann::RQNT(...): Ann " << eventID << " was asked for file  " << fileName;
	if (CheckFileExistance(fileName) == false)
	{
		mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, "File does not exist"));
		return;
	}
	state = true;
	SendToAnnModul("cr");
	mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseOK(200, ""));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::DLCX(SHP_MGCP mgcp_)
{
	BOOST_LOG_SEV(lg, trace) << "Ann::DLCX(...) for Ann " << eventID;
	state = false;
	SendToAnnModul("dl");
	mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseOK(250, ""));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Ann::CheckFileExistance(string filename_)
{
	ifstream file(init_Params->data[STARTUP::mediaPath] + "\\" + filename_);
	if (file.is_open()) { file.close(); return true; }
	else{ return false; }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::SendToAnnModul(string event_)
{// "modulName", "eventType", "clientIP", "clientPort", "serverPort", "fileName", "eventID"
	string result = "M7S2I6P5M\n";
	result += "From=mgcp\n";
	result += "To=ann\n";
	result += "EventID=mgcp" + eventID + "\n";
	result += "EventType=" + event_ + "\n";
	result += "ClientIP=" + clientIP + "\n";
	result += "ClientPort=" + clientPort + "\n";
	result += "ServerPort=" + serverPort + "\n";
	result += "FileName=" + fileName + "\n";
	net_Data->SendModul(NETDATA::ann, result);
}