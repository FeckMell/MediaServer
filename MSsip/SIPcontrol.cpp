#include "stdafx.h"
#include "SIPcontrol.h"
using namespace sip;


SIPcontrol::SIPcontrol()
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::SIPcontrol() BEGIN-END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPcontrol::Preprocessing(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Preprocessing( IPL )";
	SHP_Point found_point = FindPoint(ipl_->data["CallID"]);
	if (found_point == nullptr)
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), fatal) << "SIPcontrol::Preprocessing() ERROR : found_point == nullptr. IPL was\n" << ipl_->ipl;
		exit(-1);
	}
	else if (ipl_->data["From"] == "dtmf")
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Preprocessing() ipl_->data[\"From\"] == \"dtmf\"";
		found_point->DTMFResult(ipl_);
	}
	else if (ipl_->data["From"] == "sql")
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Preprocessing() ipl_->data[\"From\"] == \"sql\"";
		found_point->SQLResult(ipl_);
	}
	else
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), fatal) << "SIPcontrol::Preprocessing() ERROR : not this modul. IPL was\n"<<ipl_->ipl;
		exit(-1);
	}

	if (found_point->state == Point::ready) ConfProcess(found_point, false);
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Preprocessing( IPL ) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPcontrol::Preprocessing(SHP_SIP sip_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Preprocessing( SIP )";
	if (sip_->GetParam(SIP::CMD) == "INVITE")
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Preprocessing( SIP ) INVITE";
		Invite(sip_);
	}
	else if (sip_->GetParam(SIP::CMD) == "ACK")
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Preprocessing( SIP ) ACK";
		Ack(sip_);
	}
	else if (sip_->GetParam(SIP::CMD) == "BYE")
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Preprocessing( SIP ) BYE";
		Bye(sip_);
	}
	else
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), fatal) << "SIPcontrol::Preprocessing( SIP ) UNSUPPORTED COMMAND";
		sip_->ReplyClient(NET::GS(NET::OUTER::sip), sip_->ResponseBAD());
	}
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Preprocessing( SIP ) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPcontrol::Invite(SHP_SIP sip_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Invite(...)";
	string new_port = ReservePort();
	string event_id = ReserveEventID();
	string new_sdp = GenSDP(new_port, sip_);
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Invite(...) inited data:\n1)Port=" << new_port << "\n2)EventID=" << event_id << "\n3)SDP=\n" << new_sdp << "\n\nNow initing point with this params.";
	SHP_Point new_point = make_shared<Point>(Point(sip_, new_sdp, new_port, event_id));
	vecPoints.push_back(new_point);
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Invite(...) Point created and added";
	sip_->ReplyClient(NET::GS(NET::OUTER::sip), sip_->ReplyRinging());
	sip_->ReplyClient(NET::GS(NET::OUTER::sip), sip_->ReplyOK(new_sdp));
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Invite(...) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPcontrol::Bye(SHP_SIP sip_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Bye(...) with CallID" << sip_->GetParam(SIP::CallID);
	SHP_Point found_point = FindPoint(sip_->GetParam(SIP::CallID));
	if (found_point == nullptr)
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Bye(...) found_point == nullptr.";
		sip_->ResponseBAD();
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Bye(...) END";
		return;
	}
	if (found_point->state == Point::ready)
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Bye(...) found_point->state == Point::ready";
		ConfProcess(found_point, true);
	}
	else
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Bye(...) found_point->state != Point::ready";
		found_point->StopAll();
	}
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Bye(...) Removing point";
	RemovePoint(found_point);
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Bye(...) Removed point";
	sip_->ReplyClient(NET::GS(NET::OUTER::sip), sip_->ReplyOK(""));
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Bye(...) with CallID" << sip_->GetParam(SIP::CallID) << " END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPcontrol::Ack(SHP_SIP)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::Ack() BEGIN END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPcontrol::ConfProcess(SHP_Point point_, bool remove_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::ConfProcess(...)";
	SHP_Cnf found_cnf = FindCnf(point_->roomID);
	if (remove_ == true)
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::ConfProcess(...) remove_ == true";
		if (found_cnf->RmPoint(point_)) RemoveCnf(found_cnf); 
	}
	else
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::ConfProcess(...) remove_ == false";
		if (found_cnf == nullptr)
		{
			BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::ConfProcess(...) remove_ == false, found_cnf == nullptr";
			SHP_Cnf new_cnf = make_shared<Cnf>(Cnf(point_));
		}
		else
		{
			BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::ConfProcess(...) remove_ == false, found_cnf != nullptr";
			found_cnf->AddPoint(point_);
		}
	}
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPcontrol::ConfProcess(...) END";
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
SHP_Cnf SIPcontrol::FindCnf(string roomid_)
{
	for (auto &cnf : vecCnfs) if (cnf->roomID == roomid_) return cnf;
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
		//%init_Params->data[STARTUP::outerIP]
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
	//int free_port = stoi(init_Params->data[STARTUP::rtpPort])+1000;
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