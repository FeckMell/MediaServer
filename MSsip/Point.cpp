#include "stdafx.h"
#include "Point.h"

Point::Point(SHP_SIP sip_, string server_sdp_, string server_port_, string event_id_)
{
	BOOST_LOG_SEV(lg, trace) << "Point::Point(...) for CallID=" << sip_->GetParam(SIP::CallID);
	eventID = event_id_;
	serverSDP = server_sdp_;
	serverPort = server_port_;
	clientSDP = sip_->sdp;
	clientPort = GetPortFromSDP(sip_->sdp);
	clientIP = GetIPfromSDP(sip_->sdp);
	callID = sip_->GetParam(SIP::CallID);
	BOOST_LOG_SEV(lg, debug) << "CallerBase::CallerBase(...):\n1)serverSDP=\n" << serverSDP << "\n2)ServerPort=" << serverPort << "\n3)ClientSDP=\n" << clientSDP << "\n4)ClientPort=" << clientPort << "\n5)ClientIP=" << clientIP << "\n6)CallID=" << callID;
	BOOST_LOG_SEV(lg, debug) << "CallerBase::CallerBase(...): state=" << state;

	BOOST_LOG_SEV(lg, trace) << "Point::Point(...) for CallID=" << callID << " ListenDTMF()";
	ListenDTMF();
	BOOST_LOG_SEV(lg, trace) << "Point::Point(...) for CallID=" << callID << " PlayAnn(...)";
	PlayAnn("login.wav");
	BOOST_LOG_SEV(lg, trace) << "Point::Point(...) for CallID=" << callID << " END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::DTMFResult(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(lg, trace) << "Point::DTMFResult(...) for CallID=" << callID;
	if (state == login)
	{
		BOOST_LOG_SEV(lg, debug) << "Point::DTMFResult(...) for CallID=" << callID << " state==login";
		roomID = ipl_->data["Data"];
		BOOST_LOG_SEV(lg, debug) << "Point::DTMFResult(...) for CallID=" << callID << " state==login, roomID=" << roomID;
		SQLCheck("Login=" + ipl_->data["Data"]);
		state = pass;
	}
	else if (state == pass)
	{
		BOOST_LOG_SEV(lg, debug) << "Point::DTMFResult(...) for CallID=" << callID << " state==pass, pass=" << ipl_->data["Data"];
		SQLCheck("Password=" + ipl_->data["Data"]);
		state = ready;
	}
	else
	{
		BOOST_LOG_SEV(lg, fatal) << "Point::DTMFResult(...) for CallID=" << callID << " ERROR";
		exit(-1);
	}
	BOOST_LOG_SEV(lg, trace) << "Point::DTMFResult(...) for CallID=" << callID << " END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::SQLResult(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(lg, trace) << "Point::SQLResult(...) for CallID=" << callID;
	if (state == login && ipl_->data["Result"] == "false")
	{

		BOOST_LOG_SEV(lg, trace) << "Point::SQLResult(...) for CallID=" << callID << " state == login && ipl_->data[\"Result\"] == \"false\"";
		StopAnn();
		ListenDTMF();
		PlayAnn("login_again.wav");
	}
	else if (state == login && ipl_->data["Result"] == "true")
	{
		BOOST_LOG_SEV(lg, trace) << "Point::SQLResult(...) for CallID=" << callID << " state == login && ipl_->data[\"Result\"] == \"true\"";
		state = pass;
		StopAnn();
		ListenDTMF();
		PlayAnn("pass.wav");
	}
	else if (state == pass && ipl_->data["Result"] == "false")
	{
		BOOST_LOG_SEV(lg, trace) << "Point::SQLResult(...) for CallID=" << callID << " state == pass && ipl_->data[\"Result\"] == \"false\"";
		StopAnn();
		ListenDTMF();
		PlayAnn("pass_again.wav");
	}
	else if (state == pass && ipl_->data["Result"] == "true")
	{
		BOOST_LOG_SEV(lg, trace) << "Point::SQLResult(...) for CallID=" << callID << " state == pass && ipl_->data[\"Result\"] == \"true\"";
		state = ready;
		StopAnn();
	}
	else
	{
		BOOST_LOG_SEV(lg, fatal) << "Point::SQLResult(...) for CallID=" << callID << " ERROR";
		exit(-1);
	}
	BOOST_LOG_SEV(lg, trace) << "Point::SQLResult(...) for CallID=" << callID << " END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::StopAll()
{
	BOOST_LOG_SEV(lg, trace) << "Point::StopAll() for CallID=" << callID;
	StopAnn();
	StopDTMF();
	BOOST_LOG_SEV(lg, trace) << "Point::StopAll() for CallID=" << callID << " END";
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
void Point::PlayAnn(string file_name_)
{
	this_thread::sleep_for(chrono::milliseconds(100));
	BOOST_LOG_SEV(lg, trace) << "Point::PlayAnn() for CallID=" << callID << " filename=" << file_name_;
	string result = "M7S2I6P5M\n";
	result += "From=sip\n";
	result += "To=ann\n";
	result += "EventID=sip" + eventID + "\n";
	result += "EventType=cr\n";
	result += "ClientIP=" + clientIP + "\n";
	result += "ClientPort=" + clientPort + "\n";
	result += "ServerPort=" + serverPort + "\n";
	result += "FileName=" + file_name_ + "\n";
	net_Data->SendModul(NETDATA::ann, result);
	BOOST_LOG_SEV(lg, trace) << "Point::PlayAnn() for CallID=" << callID << " filename=" << file_name_<< " END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::StopAnn()
{
	BOOST_LOG_SEV(lg, trace) << "Point::StopAnn() for CallID=" << callID;
	string result = "M7S2I6P5M\n";
	result += "From=sip\n";
	result += "To=ann\n";
	result += "EventID=sip" + eventID + "\n";
	result += "EventType=dl\n";
	result += "ClientIP=" + clientIP + "\n";
	result += "ClientPort=" + clientPort + "\n";
	result += "ServerPort=" + serverPort + "\n";
	net_Data->SendModul(NETDATA::ann, result);
	BOOST_LOG_SEV(lg, trace) << "Point::StopAnn() for CallID=" << callID << " END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::ListenDTMF()
{
	BOOST_LOG_SEV(lg, trace) << "Point::ListenDTMF() for CallID=" << callID;
	string result = "M7S2I6P5M\n";
	result += "From=sip\n";
	result += "To=dtmf\n";
	result += "EventID=sip" + eventID + "\n";
	result += "EventType=cr\n";
	result += "ClientIP=" + clientIP + "\n";
	result += "ClientPort=" + clientPort + "\n";
	result += "ServerPort=" + serverPort + "\n";
	result += "CallID=" + callID + "\n";
	net_Data->SendModul(NETDATA::dtmf, result);
	BOOST_LOG_SEV(lg, trace) << "Point::ListenDTMF() for CallID=" << callID << " END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::StopDTMF()
{
	BOOST_LOG_SEV(lg, trace) << "Point::StopDTMF() for CallID=" << callID;
	string result = "M7S2I6P5M\n";
	result += "From=sip\n";
	result += "To=dtmf\n";
	result += "EventID=sip" + eventID + "\n";
	result += "EventType=dl\n";
	net_Data->SendModul(NETDATA::dtmf, result);
	BOOST_LOG_SEV(lg, trace) << "Point::StopDTMF() for CallID=" << callID << " END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::SQLCheck(string str_)
{
	BOOST_LOG_SEV(lg, trace) << "Point::SQLCheck() for CallID=" << callID;
	string result = "M7S2I6P5M\n";
	result += "From=sip\n";
	result += "To=sql\n";
	result += "EventID=sip" + eventID + "\n";
	result += "CallID=" + callID + "\n";
	result += str_ + "\n";
	net_Data->SendModul(NETDATA::sql, result);
	BOOST_LOG_SEV(lg, trace) << "Point::SQLCheck() for CallID=" << callID << " END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------