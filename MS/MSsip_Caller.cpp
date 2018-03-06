#include "stdafx.h"
#include "MSsip_Caller.h"
using namespace sip;

Caller::Caller(string call_id_, SHP_SDP c_SDP_, SHP_SDP s_SDP_)
{
	basePoint = POINTSTORE::CreatePoint(call_id_, s_SDP_);
	basePoint->ModifyClientSDP(c_SDP_);
	eventID = EVENTID::ReserveEventID();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Caller::GetParam(string param_)
{
	if (param_ == "EventID") return eventID;
	else if (param_ == "RoomID") return roomID;
	else return basePoint->GetParam(param_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::DTMFResult(SHP_IPL ipl_)
{
	StopAll();
	if (state == login)
	{

		roomID = ipl_->data["Data"];
		state = ready;
	}
	else
	{
		LOG::Log(LOG::fatal, "ERRORS", "MSSIP: Point ERROR 1");
		exit(-1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::StopAll()
{
	if (playingAnn = true) SendToAnnModul("dl", "");
	if (listenDTMF = true) SendToDTMFModul("dl");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::SendToAnnModul(string event_, string file_name_)
{
	if (playingAnn == true && event_ == "cr"){ LOG::Log(LOG::fatal, "ERRORS", "SIP playing ann ERROR 1"); }
	else if (playingAnn == false && event_ == "dl"){ LOG::Log(LOG::fatal, "ERRORS", "SIP playing ann ERROR 2"); }

	if (event_ == "cr") playingAnn = true;
	else playingAnn = false;

	string result = "";
	result += "From=sip\n";
	result += "To=ann\n";
	result += "EventID=sip" + eventID + "\n";
	result += "EventType=" + event_ + "\n";
	result += "CallID" + GetParam("CallID") + "\n";
	result += "FileName=" + file_name_ + "\n";
	COM::vecSigsIN[COM::INNER::ann](result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::SendToDTMFModul(string event_)
{
	if (listenDTMF == true && event_ == "cr"){ LOG::Log(LOG::fatal, "ERRORS", "SIP listen DTMF ERROR 1"); }
	else if (listenDTMF == false && event_ == "dl"){ LOG::Log(LOG::fatal, "ERRORS", "SIP listen DTMF ERROR 2"); }

	if (event_ == "cr") listenDTMF = true;
	else listenDTMF = false;

	string result = "";
	result += "From=sip\n";
	result += "To=dtmf\n";
	result += "EventID=sip" + eventID + "\n";
	result += "EventType=" + eventID + "\n";
	result += "CallID" + GetParam("CallID") + "\n";
	COM::vecSigsIN[COM::INNER::dtmf](result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
