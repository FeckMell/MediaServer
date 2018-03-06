#include "stdafx.h"
#include "Ann.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Ann::Ann(SHP_MGCP mgcp_, string server_sdp_, string server_port_)
:CallerBase(mgcp_, server_sdp_, server_port_)
{
	eventNum = mgcp_->data[MGCP::EventNum];
	ReplyClient(mgcp_, mgcp_->ResponseOK(200,"add event type") + "\n\n" + server_sdp_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::RQNT(SHP_MGCP mgcp_)
{
	fileName = get_substr(mgcp_->data[MGCP::ParamS], ",file:///", ")");
	if (CheckFileExistance(fileName) == false)
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "File does not exist"));
		return;
	}
	state = true;
	SendToAnnModul("cr");
	ReplyClient(mgcp_, mgcp_->ResponseOK(200, ""));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::DLCX(SHP_MGCP mgcp_)
{
	state = false;
	SendToAnnModul("dl");
	ReplyClient(mgcp_, mgcp_->ResponseOK(250, ""));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Ann::CheckFileExistance(string filename_)
{
	ifstream file(init_Params->data[IPar::PathEXE] + init_Params->data[IPar::MediaPath] + "\\" + filename_);
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
	SendModul(NETDATA::ann, result);
}