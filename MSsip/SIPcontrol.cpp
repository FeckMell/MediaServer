#include "stdafx.h"
#include "SIPcontrol.h"
using namespace sip;


SIPcontrol::SIPcontrol()
{
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPcontrol::Preprocessing(SHP_IPL ipl_)
{
	SHP_Point found_point = FindPoint(ipl_->data["CallID"]);
	if (found_point == nullptr)
	{
		exit(-1);
	}
	else if (ipl_->data["From"] == "dtmf")
	{
		found_point->DTMFResult(ipl_);
	}
	else if (ipl_->data["From"] == "sql")
	{
		found_point->SQLResult(ipl_);
	}
	else
	{
		exit(-1);
	}

	if (found_point->state == Point::ready) ConfProcess(found_point, false);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPcontrol::Preprocessing(SHP_SIP sip_)
{
	if (sip_->GetParam(SIP::CMD) == "INVITE")
	{
		Invite(sip_);
	}
	else if (sip_->GetParam(SIP::CMD) == "ACK")
	{
		Ack(sip_);
	}
	else if (sip_->GetParam(SIP::CMD) == "BYE")
	{
		Bye(sip_);
	}
	else
	{
		sip_->ReplyClient(NET::GS(NET::OUTER::sip), sip_->ResponseBAD());
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPcontrol::Invite(SHP_SIP sip_)
{
	string new_port = ReservePort();
	string event_id = ReserveEventID();
	string new_sdp = GenSDP(new_port, sip_);

	SHP_Point new_point = make_shared<Point>(Point(sip_, new_sdp, new_port, event_id));
	vecPoints.push_back(new_point);

	sip_->ReplyClient(NET::GS(NET::OUTER::sip), sip_->ReplyRinging());
	sip_->ReplyClient(NET::GS(NET::OUTER::sip), sip_->ReplyOK(new_sdp));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPcontrol::Bye(SHP_SIP sip_)
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
	sip_->ReplyClient(NET::GS(NET::OUTER::sip), sip_->ReplyOK(""));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPcontrol::Ack(SHP_SIP)
{
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPcontrol::ConfProcess(SHP_Point point_, bool remove_)
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
SHP_Point SIPcontrol::FindPoint(string id_)
{
	for (auto &point : vecPoints) if (point->callID == id_) return point;
	return nullptr;	
}
void  SIPcontrol::RemovePoint(SHP_Point point_)
{
	vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
	FreeEventID(point_->eventID);
}
SHP_Cnf SIPcontrol::FindCnf(string room_id_)
{
	for (auto &cnf : vecCnfs) if (cnf->roomID == room_id_) return cnf;
	return nullptr;
}
void SIPcontrol::RemoveCnf(SHP_Cnf cnf_)
{
	vecCnfs.erase(std::remove(vecCnfs.begin(), vecCnfs.end(), cnf_), vecCnfs.end());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string SIPcontrol::GenSDP(string port_s_, SHP_SIP sip_)
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
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string SIPcontrol::ReservePort()
{
	int free_port = stoi(CFG::data[CFG::rtpPort]) + 1000;
	if (usedPorts.size() == 0)
	{
		usedPorts.push_back(free_port);
		return to_string(free_port);
	}
	for (unsigned i = 0; i < usedPorts.size(); ++i)
	{
		if (usedPorts[i] != free_port)
		{
			usedPorts.push_back(free_port);
			sort(usedPorts.begin(), usedPorts.end());
			return to_string(free_port);
		}
		free_port += 2;
	}
	usedPorts.push_back(free_port);
	sort(usedPorts.begin(), usedPorts.end());
	return to_string(free_port);
}
void SIPcontrol::FreePort(string port_)
{
	usedPorts.erase(remove(usedPorts.begin(), usedPorts.end(), stoi(port_)), usedPorts.end());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string SIPcontrol::ReserveEventID()
{
	int free_event_id = 0;
	if (usedEventID.size() == 0) { usedEventID.push_back(free_event_id);  return to_string(free_event_id); }
	for (unsigned i = 0; i < usedEventID.size(); ++i)
	{
		if (usedEventID[i] != free_event_id)
		{
			usedEventID.push_back(free_event_id);
			sort(usedEventID.begin(), usedEventID.end());
			return to_string(free_event_id);
		}
		free_event_id++;
	}
	usedEventID.push_back(free_event_id);
	sort(usedEventID.begin(), usedEventID.end());
	return to_string(free_event_id);
}
void SIPcontrol::FreeEventID(string event_id_)
{
	usedEventID.erase(remove(usedEventID.begin(), usedEventID.end(), stoi(event_id_)), usedEventID.end());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------