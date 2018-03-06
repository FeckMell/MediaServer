#include "stdafx.h"
#include "Ann.h"
using namespace mgcp;


Ann::Ann(SHP_Point point_, SHP_MGCP mgcp_) : point(point_), eventID(mgcp_->data["EventID"])
{
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::Delete()
{
	
	if (state == true){ SendToAnnModul("dl"); }
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::RequestMusic(SHP_MGCP mgcp_)
{
	
	fileName = get_substr(mgcp_->data["S"], ",file:///", ")");
	
	if (CheckFileExistance() == false)
	{
		
		mgcp_->innerError = "File does not exist";
		return;
	}
	state = true;
	SendToAnnModul("cr");
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Ann::CheckFileExistance()
{
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
	
}