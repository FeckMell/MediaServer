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
	BOOST_LOG_SEV(LOG::GL(0), info) << "MSSIP: ipl:\n" << message_;

	SHP_Point found_point = FindPoint(ipl->data["CallID"]);
	if (found_point == nullptr)
	{
		BOOST_LOG_SEV(LOG::GL(0), info) << "MSSIP: Preproc ERROR 1";
		exit(-1);
	}
	else if (ipl->data["From"] == "dtmf")
	{
		found_point->DTMFResult(ipl);
	}
	else if (ipl->data["From"] == "sql")
	{
		BOOST_LOG_SEV(LOG::GL(0), info) << "MSSIP: Preproc ERROR 3";
	}
	else
	{
		BOOST_LOG_SEV(LOG::GL(0), info) << "MSSIP: Preproc ERROR 2";
		exit(-1);
	}

	if (found_point->state == Point::ready) ConfProcess(found_point, false);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::PreprocessingOUT(REQUEST message_)
{
	SHP_SIP sip = make_shared<SIP>(SIP(message_.data, message_.sender));
	BOOST_LOG_SEV(LOG::GL(0), info) << "MSSIP: Preproc sip:\n" << message_.data;

	if (sip->outerError == "")
	{
		if (sip->data["CMD"] == "INVITE")
		{
			Invite(sip);
			sip->ReplyClient();
		}
		else if (sip->data["CMD"] == "ACK")
		{
			Ack(sip);
		}
		else if (sip->data["CMD"] == "BYE")
		{
			Bye(sip);
			sip->ReplyClient();
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Invite(SHP_SIP sip_)
{
	string new_port = SSTORAGE::ReservePort();
	string event_id = SSTORAGE::ReserveEventID();
	sip_->serverSDP.reset(new SDP(new_port, sip_->data["CallID"]));

	SHP_Point new_point = make_shared<Point>(Point(sip_, event_id));
	vecPoints.push_back(new_point);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Bye(SHP_SIP sip_)
{
	SHP_Point found_point = FindPoint(sip_->data["CallID"]);
	if (found_point == nullptr)
	{
		sip_->innerError = "Bye: not found point";
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
		if (found_cnf->RmPoint(point_) == true) RemoveCnf(found_cnf);
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
SHP_Cnf Control::FindCnf(string event_id_)
{
	for (auto& cnf : vecCnfs) if (cnf->eventID == event_id_) return cnf;
	return nullptr;
}
void Control::RemoveCnf(SHP_Cnf cnf_)
{
	vecCnfs.erase(std::remove(vecCnfs.begin(), vecCnfs.end(), cnf_), vecCnfs.end());
}
