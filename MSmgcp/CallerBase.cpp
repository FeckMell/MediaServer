#include "stdafx.h"
#include "CallerBase.h"

CallerBase::CallerBase(SHP_MGCP mgcp_, string server_sdp_, string server_port_)
{
	BOOST_LOG_SEV(lg, trace) << "CallerBase::CallerBase(...) for CallID=" << mgcp_->data[MGCP::ParamC];
	serverSDP = server_sdp_;
	serverPort = server_port_;
	serverIP = init_Params->data[STARTUP::outerIP];
	clientSDP = mgcp_->data[MGCP::SDP];
	clientPort = GetPortFromSDP(mgcp_->data[MGCP::SDP]);
	clientIP = GetIPfromSDP(mgcp_->data[MGCP::SDP]);
	callID = mgcp_->data[MGCP::ParamC];
	BOOST_LOG_SEV(lg, debug) << "CallerBase::CallerBase(...):\n1)serverSDP=\n" << serverSDP << "\n2)ServerPort=" << serverPort << "\n3)ServerIP=" << serverIP << "\n4)ClientSDP=\n" << clientSDP << "\n5)ClientPort=" << clientPort << "\n6)ClientIP=" << clientIP << "\n7)CallID=" << callID;
	if (FindSDPmode(clientSDP) == "sendrecv") state = true;
	else state = false;
	BOOST_LOG_SEV(lg, debug) << "CallerBase::CallerBase(...): state=" << state;
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
	BOOST_LOG_SEV(lg, trace) << "CallerBase::ModifyCallerBase(...) for CallID=" << callID;
	if (clientSDP == "" && mgcp_->data[MGCP::SDP] != "") 
	{
		BOOST_LOG_SEV(lg, trace) << "if (clientSDP == \"\" && mgcp_->data[MGCP::SDP] != \"\"):";
		clientSDP = mgcp_->data[MGCP::SDP];
		clientPort = GetPortFromSDP(mgcp_->data[MGCP::SDP]);
		clientIP = GetIPfromSDP(mgcp_->data[MGCP::SDP]);
		state = true;
		BOOST_LOG_SEV(lg, debug) << "1)ClientSDP=\n" << clientSDP << "\n2)ClientPort=" << clientPort << "\n3)ClientIP=" << clientIP << "\n4)State=" << state << "\n5)Will return \"0\"";
		return "0";
	}
	if (clientSDP != "" && mgcp_->data[MGCP::SDP] != "")
	{
		BOOST_LOG_SEV(lg, trace) << "if (clientSDP != \"\" && mgcp_->data[MGCP::SDP] != \"\"): Call find and change mode.";
		string stored_mode = FindSDPmode(clientSDP);
		string new_mode = FindSDPmode(mgcp_->data[MGCP::SDP]);
		BOOST_LOG_SEV(lg, debug) << "Stored mode=" << stored_mode << " new_mode=" << new_mode;
		if (stored_mode == "error" || new_mode == "error") return "error mode";
		if (stored_mode == new_mode) return "error: same modes";
		if (stored_mode != new_mode)
		{
			clientSDP = ChangeSDPmode(clientSDP, stored_mode, new_mode);
			state = !state;
			BOOST_LOG_SEV(lg, debug) << "clientSDP=\n" << clientSDP << "\nState=" << state<<"\nWill return \"1\"";
			return "1";
		}
	}
	if (clientSDP == "" && mgcp_->data[MGCP::SDP] == "")
	{
		BOOST_LOG_SEV(lg, fatal) << "if (clientSDP == \"\" && mgcp_->data[MGCP::SDP] == \"\"): will return \"error: No SDPs\"";
		return "error: No SDPs";
	}
	if (clientSDP != "" && mgcp_->data[MGCP::SDP] == "")
	{
		BOOST_LOG_SEV(lg, fatal) << "if (clientSDP != \"\" && mgcp_->data[MGCP::SDP] == \"\"): will return \"error: what to modify?\"";
		return "error: what to modify?";
	}
	BOOST_LOG_SEV(lg, fatal) << "CallerBase::ModifyCallerBase(...): default error";
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
	BOOST_LOG_SEV(lg, fatal) << "CallerBase::FindSDPmode(...): not found";
	return "error";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string CallerBase::ChangeSDPmode(string sdp_, string what_, string to_what_)
{
	sdp_.replace(sdp_.find(what_), what_.length(), to_what_);
	return sdp_;
}