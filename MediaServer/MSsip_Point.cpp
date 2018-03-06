#include "stdafx.h"
#include "MSsip_Point.h"
using namespace sip;


Point::Point(SHP_SIP sip_, string server_sdp_, string server_port_, string event_id_)
{
	
	eventID = event_id_;
	serverSDP = server_sdp_;
	serverPort = server_port_;
	clientSDP = sip_->sdp;
	clientPort = GetPortFromSDP(sip_->sdp);
	clientIP = GetIPfromSDP(sip_->sdp);
	callID = sip_->GetParam(SIP::CallID);
	
	

	
	ListenDTMF();
	
	PlayAnn("login.wav");
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::DTMFResult(SHP_IPL ipl_)
{
	
	if (state == login)
	{
		
		roomID = ipl_->data["Data"];
		
		SQLCheck("Login=" + ipl_->data["Data"]);
		//state = pass;
		state = ready;
	}
	else if (state == pass)
	{
		
		SQLCheck("Password=" + ipl_->data["Data"]);
		state = ready;
	}
	else
	{
		
		exit(-1);
	}
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::SQLResult(SHP_IPL ipl_)
{
	
	if (state == login && ipl_->data["Result"] == "false")
	{

		
		StopAnn();
		ListenDTMF();
		PlayAnn("login_again.wav");
	}
	else if (state == login && ipl_->data["Result"] == "true")
	{
		
		state = pass;
		StopAnn();
		ListenDTMF();
		PlayAnn("pass.wav");
	}
	else if (state == pass && ipl_->data["Result"] == "false")
	{
		
		StopAnn();
		ListenDTMF();
		PlayAnn("pass_again.wav");
	}
	else if (state == pass && ipl_->data["Result"] == "true")
	{
		
		state = ready;
		StopAnn();
	}
	else
	{
		
		exit(-1);
	}
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::StopAll()
{
	
	StopAnn();
	StopDTMF();
	
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
	
	string result = "";
	result += "From=sip\n";
	result += "To=ann\n";
	result += "EventID=sip" + eventID + "\n";
	result += "EventType=cr\n";
	result += "ClientIP=" + clientIP + "\n";
	result += "ClientPort=" + clientPort + "\n";
	result += "ServerPort=" + serverPort + "\n";
	result += "FileName=" + file_name_ + "\n";
	NET::vecSigsIN[NET::INNER::ann](result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::StopAnn()
{
	
	string result = "";
	result += "From=sip\n";
	result += "To=ann\n";
	result += "EventID=sip" + eventID + "\n";
	result += "EventType=dl\n";
	result += "ClientIP=" + clientIP + "\n";
	result += "ClientPort=" + clientPort + "\n";
	result += "ServerPort=" + serverPort + "\n";
	NET::vecSigsIN[NET::INNER::ann](result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::ListenDTMF()
{
	
	string result = "";
	result += "From=sip\n";
	result += "To=dtmf\n";
	result += "EventID=sip" + eventID + "\n";
	result += "EventType=cr\n";
	result += "ClientIP=" + clientIP + "\n";
	result += "ClientPort=" + clientPort + "\n";
	result += "ServerPort=" + serverPort + "\n";
	result += "CallID=" + callID + "\n";
	NET::vecSigsIN[NET::INNER::dtmf](result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::StopDTMF()
{
	
	string result = "";
	result += "From=sip\n";
	result += "To=dtmf\n";
	result += "EventID=sip" + eventID + "\n";
	result += "EventType=dl\n";
	NET::vecSigsIN[NET::INNER::dtmf](result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::SQLCheck(string str_)
{
	
	string result = "";
	result += "From=sip\n";
	result += "To=sql\n";
	result += "EventID=sip" + eventID + "\n";
	result += "CallID=" + callID + "\n";
	result += str_ + "\n";
	NET::vecSigsIN[NET::INNER::sql](result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------