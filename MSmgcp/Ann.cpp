#include "Ann.h"

Ann::Ann(SHP_MGCP mgcp_, string server_sdp_, string server_port_)
:CallerBase(mgcp_, server_sdp_, server_port_)
{
	BOOST_LOG_SEV(lg, trace) << "Ann::Ann(...)";
	eventNum = mgcp_->data[MGCP::EventNum];
	BOOST_LOG_SEV(lg, trace) << "Set this Ann eventNum=" << eventNum;
	//ReplyClient(mgcp_, mgcp_->ResponseOK(200,"add event type") + "\n\n" + server_sdp_);
	mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseOK(200, "add event type") + "\n\n" + server_sdp_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::RQNT(SHP_MGCP mgcp_)
{
	BOOST_LOG_SEV(lg, trace) << "Ann::RQNT(...) for Ann " << eventNum;
	fileName = get_substr(mgcp_->data[MGCP::ParamS], ",file:///", ")");
	BOOST_LOG_SEV(lg, info) << "Ann::RQNT(...): Ann " << eventNum << " was asked for file  " << fileName;
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
	BOOST_LOG_SEV(lg, trace) << "Ann::DLCX(...) for Ann " << eventNum;
	state = false;
	SendToAnnModul("dl");
	mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseOK(250, ""));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Ann::CheckFileExistance(string filename_)
{
	ifstream file(init_Params->data[IPar::mediaPath] + "\\" + filename_);
	if (file.is_open()) { file.close(); return true; }
	else{ return false; }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::SendToAnnModul(string event_)
{// "modulName", "eventType", "clientIP", "clientPort", "serverPort", "fileName", "eventID"
	string result = "modulName=ann\n";
	result += "eventType=" + event_+"\n";
	result += "clientIP=" + clientIP + "\n";
	result += "clientPort=" + clientPort + "\n";
	result += "serverPort=" + serverPort + "\n";
	result += "fileName=" + fileName + "\n";
	result += "eventID=" + eventNum + "\n";
	net_Data->SendModul(NETDATA::ann, result);
}