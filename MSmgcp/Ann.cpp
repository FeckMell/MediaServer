#include "stdafx.h"
#include "Ann.h"
using namespace mgcp;


Ann::Ann(SHP_Point point_, SHP_MGCP mgcp_) : point(point_), eventID(mgcp_->data["EventID"])
{
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Ann::Ann with eventID=" << eventID;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::Delete()
{
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Ann::Delete with eventID=" << eventID;
	if (state == true){ SendToAnnModul("dl"); }
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Ann::Delete END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::RequestMusic(SHP_MGCP mgcp_)
{
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Ann::RequestMusic with eventID=" << eventID;
	fileName = get_substr(mgcp_->data["S"], ",file:///", ")");
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Ann::RequestMusic fileName=" << fileName;
	if (CheckFileExistance() == false)
	{
		BOOST_LOG_SEV(LOG::vecLogs, warning) << "Ann::RequestMusic fileName=" << fileName << " ERROR";
		mgcp_->innerError = "File does not exist";
		return;
	}
	state = true;
	SendToAnnModul("cr");
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Ann::RequestMusic END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Ann::CheckFileExistance()
{
	//ifstream file(init_Params->data[STARTUP::mediaPath] + "\\" + fileName);
	ifstream file(CFG::data[CFG::mediaPath] + "\\" + fileName);
	if (file.is_open()) { file.close(); return true; }
	else{ return false; }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::SendToAnnModul(string event_)
{
	string result = "M7S2I6P5M\n";
	result += "From=mgcp\n";
	result += "To=ann\n";
	result += "EventID=mgcp" + eventID + "\n";
	result += "EventType=" + event_ + "\n";
	result += "ClientIP=" + point->clientIP + "\n";
	result += "ClientPort=" + point->clientPort + "\n";
	result += "ServerPort=" + point->serverPort + "\n";
	result += "FileName=" + fileName + "\n";
	NET::SendModul(NET::INNER::mgcp_i, NET::INNER::ann, result);
	//net_Data->SendModul(NETDATA::ann, result);
	BOOST_LOG_SEV(LOG::vecLogs, debug) << "Ann::SendToAnnModul with eventID=" << eventID << " sent:\n" << result;
}