#include "stdafx.h"
#include "CallerBase.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
CallerBase::CallerBase(SHP_MGCP mgcp_, string server_sdp_, string server_port_)
{
	server_SDP = server_sdp_;
	server_Port = server_port_;
	server_IP = init_Params->data[InitParams::outerIP];
	client_SDP = mgcp_->data[MGCP::SDP];
	client_Port = GetIPfromSDP(mgcp_->data[MGCP::SDP]);
	client_IP = GetPortFromSDP(mgcp_->data[MGCP::SDP]);
	callID = mgcp_->data[MGCP::ParamC];
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
	if (client_SDP == "" && mgcp_->data[MGCP::SDP] != "") 
	{
		client_SDP = mgcp_->data[MGCP::SDP];
		client_Port = GetIPfromSDP(mgcp_->data[MGCP::SDP]);
		client_IP = GetPortFromSDP(mgcp_->data[MGCP::SDP]);
		state = "ready";
		return "0";
	}
	if (client_SDP != "" && mgcp_->data[MGCP::SDP] != "")
	{
		string stored_mode = FindSDPmode(client_SDP);
		string mgcp_mode = FindSDPmode(mgcp_->data[MGCP::SDP]);
		if (stored_mode == "error" || mgcp_mode == "error") return "error mode";
		if (stored_mode == mgcp_mode) return "error: same modes";
		if (stored_mode != mgcp_mode)
		{
			client_SDP = ChangeSDPmode(client_SDP, stored_mode, mgcp_mode);
			return "1";
		}
	}
	if (client_SDP == "" && mgcp_->data[MGCP::SDP] == "")
	{
		return "error: No SDPs";
	}
	if (client_SDP != "" && mgcp_->data[MGCP::SDP] == "")
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