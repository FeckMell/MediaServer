#include "stdafx.h"
#include "MSsip.h"
using namespace sip;


Control::Control()
{

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::PreprocessingINNER(string message_)
{
	SHP_IPL ipl = make_shared<IPL>(IPL(message_));

	SHP_Caller found_caller = FindCaller(ipl->data["CallID"]);
	if (found_caller == nullptr)
	{
		LOG::Log(LOG::fatal, "ERRORS", "MSSIP: Preproc ERROR 1. IPL=\n" + ipl->ipl);
		exit(-1);
	}
	else if (ipl->data["From"] == "dtmf")
	{
		found_caller->DTMFResult(ipl);
	}
	else if (ipl->data["From"] == "sql")
	{
		LOG::Log(LOG::fatal, "ERRORS", "MSSIP: Preproc ERROR 3. IPL=\n" + ipl->ipl);
	}
	else
	{
		LOG::Log(LOG::fatal, "ERRORS", "MSSIP: Preproc ERROR 2. IPL=\n" + ipl->ipl);
		exit(-1);
	}

	if (found_caller->state == Caller::ready) ConfProcess(found_caller, false);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::PreprocessingOUTTER()
{
	SHP_SIP sip = make_shared<SIP>(SIP(true));

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
	SHP_Caller found_caller = FindCaller(sip_->data["CallID"]);
	if (found_caller != nullptr)
	{
		sip_->innerError = "Invite: point already exists";
		return;
	}
	
	sip_->serverSDP.reset(new SDP(SOCKSTORE::ReservePort(), sip_->data["CallID"]));

	SHP_Caller new_point = make_shared<Caller>(Caller(sip_->data["CallID"], sip_->clientSDP, sip_->serverSDP));
	vecCallers.push_back(new_point);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Bye(SHP_SIP sip_)
{
	SHP_Caller found_caller = FindCaller(sip_->data["CallID"]);
	if (found_caller == nullptr)
	{
		sip_->innerError = "Bye: not found point";
		return;
	}
	if (found_caller->state == Caller::ready)
	{
		ConfProcess(found_caller, true);
	}
	else
	{
		found_caller->StopAll();
	}
	RemoveCaller(found_caller);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Ack(SHP_SIP)
{
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::ConfProcess(SHP_Caller caller_, bool remove_)
{
	SHP_Cnf found_cnf = FindCnf(caller_->GetParam("RoomID"));
	if (remove_ == true)
	{
		if (found_cnf->RmPoint(caller_) == true) RemoveCnf(found_cnf);
	}
	else
	{
		if (found_cnf == nullptr)
		{
			SHP_Cnf new_cnf = make_shared<Cnf>(Cnf(caller_));
			vecCnfs.push_back(new_cnf);
		}
		else
		{
			found_cnf->AddPoint(caller_);
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Caller Control::FindCaller(string id_)
{
	for (auto& e : vecCallers) if (e->GetParam("CallID") == id_) return e;
	return nullptr;
}
void  Control::RemoveCaller(SHP_Caller point_)
{
	vecCallers.erase(std::remove(vecCallers.begin(), vecCallers.end(), point_), vecCallers.end());
	POINTSTORE::DeletePoint(point_->GetParam("CallID"));
	EVENTID::FreeEventID(point_->GetParam("EventID"));
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
