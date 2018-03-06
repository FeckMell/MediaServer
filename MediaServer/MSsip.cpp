#include "stdafx.h"
#include "MSsip.h"
using namespace sip;


Control::Control()
{

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::PreprocessingIN(string message_)
{
	SHP_IPL ipl = make_shared<IPL>(IPL(message_));

	SHP_Point found_point = FindPoint(ipl->data["CallID"]);
	if (found_point == nullptr)
	{
		exit(-1);
	}
	else if (ipl->data["From"] == "dtmf")
	{
		found_point->DTMFResult(ipl);
	}
	else if (ipl->data["From"] == "sql")
	{
		found_point->SQLResult(ipl);
	}
	else
	{
		exit(-1);
	}

	if (found_point->state == Point::ready) ConfProcess(found_point, false);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::PreprocessingOUT(REQUEST message_)
{
	SHP_SIP sip = make_shared<SIP>(SIP(message_.data, message_.sender));

	if (sip->error == "")
	{
		if (sip->GetParam(SIP::CMD) == "INVITE")
		{
			Invite(sip);
		}
		else if (sip->GetParam(SIP::CMD) == "ACK")
		{
			Ack(sip);
		}
		else if (sip->GetParam(SIP::CMD) == "BYE")
		{
			Bye(sip);
		}
		else
		{
			sip->ReplyClient(sip->ResponseBAD());
		}
	}
	else
	{
		sip->ReplyClient(sip->ResponseBAD());
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Invite(SHP_SIP sip_)
{
	string new_port = SSTORAGE::ReservePort();
	string event_id = SSTORAGE::ReserveEventID();
	string new_sdp = GenSDP(new_port, sip_);

	SHP_Point new_point = make_shared<Point>(Point(sip_, new_sdp, new_port, event_id));
	vecPoints.push_back(new_point);

	sip_->ReplyClient(sip_->ReplyRinging());
	sip_->ReplyClient(sip_->ReplyOK(new_sdp));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Bye(SHP_SIP sip_)
{
	SHP_Point found_point = FindPoint(sip_->GetParam(SIP::CallID));
	if (found_point == nullptr)
	{
		sip_->ResponseBAD();
		return;
	}
	if (found_point->state == Point::ready)
	{
		ConfProcess(found_point, true);
	}
	else
	{
		found_point->StopAll();
	}
	RemovePoint(found_point);
	sip_->ReplyClient(sip_->ReplyOK(""));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Ack(SHP_SIP)
{
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::ConfProcess(SHP_Point point_, bool remove_)
{
	SHP_Cnf found_cnf = FindCnf(point_->roomID);
	if (remove_ == true)
	{
		if (found_cnf->RmPoint(point_)) RemoveCnf(found_cnf); 
	}
	else
	{
		if (found_cnf == nullptr)
		{
			SHP_Cnf new_cnf = make_shared<Cnf>(Cnf(point_));
			vecCnfs.push_back(new_cnf);
		}
		else
		{
			found_cnf->AddPoint(point_);
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Point Control::FindPoint(string id_)
{
	for (auto &point : vecPoints) if (point->callID == id_) return point;
	return nullptr;	
}
void  Control::RemovePoint(SHP_Point point_)
{
	vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
	SSTORAGE::FreeEventID(point_->eventID);
}
SHP_Cnf Control::FindCnf(string room_id_)
{
	for (auto &cnf : vecCnfs) if (cnf->roomID == room_id_) return cnf;
	return nullptr;
}
void Control::RemoveCnf(SHP_Cnf cnf_)
{
	vecCnfs.erase(std::remove(vecCnfs.begin(), vecCnfs.end(), cnf_), vecCnfs.end());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Control::GenSDP(string port_s_, SHP_SIP sip_)
{
	auto template_sdp = boost::format(string(
		"v=0\n"
		"o=- %3% 0 IN IP4 %1%\n"//3,1
		"s=%4%\n"//4
		"c=IN IP4 %1%\n"//1
		"t=0 0\n"
		"a=tool:libavformat 57.3.100\n"
		"m=audio %2% RTP/AVP 8 101\n"//2
		"a=rtpmap:8 PCMA/8000\n"
		"a=rtpmap:101 telephone-event/8000\n"
		"a=ptime:20\n"
		"a=sendrecv\n"
		)); // формируем тип ответа
	auto result = str(template_sdp
		%CFG::data[CFG::outerIP]
		% port_s_
		%lastSDP_ID
		%sip_->GetParam(SIP::CallID)
		);
	lastSDP_ID++;
	return result;
}