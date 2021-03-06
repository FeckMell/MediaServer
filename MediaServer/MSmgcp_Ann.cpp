#include "stdafx.h"
#include "MSmgcp_Ann.h"
using namespace mgcp;


Ann::Ann(SHP_Point point_, SHP_MGCP mgcp_) : point(point_), eventID(mgcp_->data["EventID"])
{
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::Stop()
{
	LOG::Log(LOG::info, "MGCP", "MSMGCP: Ann Stop with id=" + eventID);
	if (state == true) 
	{ 
		state = false;
		SendToAnnModul("dl");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::RequestMusic(SHP_MGCP mgcp_)
{
	fileName = get_substr(mgcp_->data["S"], ",file:///", ")");
	if (CheckFileExistance() == false)
	{
		mgcp_->innerError = "File=" + fileName + " does not exist";
		return;
	}
	loop = mgcp_->data["Q"];
	if (point->state == true)
	{
		state = true;
		SendToAnnModul("cr");
	}
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Ann::CheckFileExistance()
{
	ifstream file(CFG::data["mediaPath"] + CFG::slash + fileName);
	if (file.is_open()) { file.close(); return true; }
	else{ return false; }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::SendToAnnModul(string event_)
{
	string result = "";
	result += "From=mgcp\n";
	result += "To=ann\n";
	result += "EventID=mgcp" + eventID + "\n";
	result += "CallID=" + point->callID + "\n";
	result += "EventType=" + event_ + "\n";
	result += "ClientIP=" + point->clientSDP->data["IP"] + "\n";
	result += "ClientPort=" + point->clientSDP->data["Port"] + "\n";
	result += "ServerPort=" + point->serverSDP->data["Port"] + "\n";
	result += "FileName=" + fileName + "\n";
	result += "Loop=" + loop + "\n";
	NET::vecSigsIN[NET::INNER::ann](result);
	
}