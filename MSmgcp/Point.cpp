#include "stdafx.h"
#include "Point.h"
using namespace mgcp;


Point::Point(SHP_MGCP mgcp_)
{
	serverSDP = mgcp_->serverSDP;
	clientSDP = mgcp_->sdp;
	serverPort= GetPortFromSDP(serverSDP);
	clientPort = GetPortFromSDP(clientSDP);
	clientIP = GetIPfromSDP(clientSDP);
	callID = mgcp_->data["CallID"];
	if (clientSDP != "") state = true;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::ModifyPoint(SHP_MGCP mgcp_)
{
	if (clientSDP == "" && mgcp_->sdp != "")
	{
		clientSDP = mgcp_->sdp;
		clientPort = GetPortFromSDP(clientSDP);
		clientIP = GetIPfromSDP(clientSDP);
		state = true;
	}
	else if (clientSDP != "" && mgcp_->sdp != "")
	{
		state = ChangeSDPmode(mgcp_->sdp);
		mgcp_->serverSDP = serverSDP;
	}
	else 
	{
		mgcp_->innerError = "Point::ModifyPoint SDP state ERROR";
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Point::GetIPfromSDP(string sdp_)
{
	return get_substr(sdp_, "c=IN IP4 ", "\n");
}
string Point::GetPortFromSDP(string sdp_)
{
	return get_substr(sdp_, "m=audio ", " ");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Point::ChangeSDPmode(string new_client_sdp_)
{
	string old_server_mode = FindSDPmode(serverSDP);
	string old_client_mode = FindSDPmode(clientSDP);
	string new_client_mode = FindSDPmode(new_client_sdp_);

	if (old_server_mode == "error" || old_client_mode == "error" || new_client_mode == "error") return state;
	if (old_client_mode == new_client_mode) return state;

	clientSDP = new_client_sdp_;
	serverSDP = serverSDP.replace(serverSDP.find(old_server_mode), old_server_mode.length(), new_client_mode);
	return !state;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Point::FindSDPmode(string sdp_)
{
	vector<string> alphabet = { "a=sendrecv", "a=inactive" };
	for (auto& e : alphabet)
	{
		if (sdp_.find(e) != string::npos)
			return e;
	}
	
	return "error";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Point::PrintPoint()
{
	string result = "\n==========================================================================================================\n";
	result += "==========================================================================================================\n";
	result += "\nCallID=" + callID + "\n";
	result += "EventID=" + eventID + "\n";
	result += "\nclientSDP=" + clientSDP + "\n";
	result += "\nserverSDP=" + serverSDP + "\n";
	result += "\nclientPort=" + clientPort + "\n";
	result += "\nserverPort=" + serverPort + "\n";
	result += "\nclientIP=" + clientIP + "\n";
	result += "==========================================================================================================\n\n\n";
	return result;
}