#include "stdafx.h"
#include "CallerBase.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
CallerBase::CallerBase(SHP_MGCP mgcp_, string server_sdp_, string server_port_)
{
	serverSDP = server_sdp_;
	serverPort = server_port_;
	serverIP = init_Params->data[IPar::outerIP];
	clientSDP = mgcp_->data[MGCP::SDP];
	clientPort = GetPortFromSDP(mgcp_->data[MGCP::SDP]);
	clientIP = GetIPfromSDP(mgcp_->data[MGCP::SDP]);
	callID = mgcp_->data[MGCP::ParamC];
	if (FindSDPmode(clientSDP) == "sendrecv") state = true;
	else state = false;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string CallerBase::GetIPfromSDP(string sdp_)
{
	return get_substr(sdp_, "c=IN IP4 ", "\n");
}
string CallerBase::GetPortFromSDP(string sdp_)
{
	return get_substr(sdp_, "m=audio ", " ");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string CallerBase::ModifyCallerBase(SHP_MGCP mgcp_)
{
	if (clientSDP == "" && mgcp_->data[MGCP::SDP] != "") 
	{
		clientSDP = mgcp_->data[MGCP::SDP];
		clientPort = GetPortFromSDP(mgcp_->data[MGCP::SDP]);
		clientIP = GetIPfromSDP(mgcp_->data[MGCP::SDP]);
		state = true;
		return "0";
	}
	if (clientSDP != "" && mgcp_->data[MGCP::SDP] != "")
	{
		string stored_mode = FindSDPmode(clientSDP);
		string new_mode = FindSDPmode(mgcp_->data[MGCP::SDP]);
		if (stored_mode == "error" || new_mode == "error") return "error mode";
		if (stored_mode == new_mode) return "error: same modes";
		if (stored_mode != new_mode)
		{
			clientSDP = ChangeSDPmode(clientSDP, stored_mode, new_mode);
			state = !state;
			return "1";
		}
	}
	if (clientSDP == "" && mgcp_->data[MGCP::SDP] == "")
	{
		return "error: No SDPs";
	}
	if (clientSDP != "" && mgcp_->data[MGCP::SDP] == "")
	{
		return "error: what to modify?";
	}
	return "error";//
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string CallerBase::FindSDPmode(string sdp_)
{
	vector<string> alphabet = { "sendrecv", "inactive" };
	for (auto& e : alphabet)
	{
		if (sdp_.find(e) != string::npos)
			return e;
	}
	return "error";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string CallerBase::ChangeSDPmode(string sdp_, string what_, string to_what_)
{
	sdp_.replace(sdp_.find(what_), what_.length(), to_what_);
	return sdp_;
}