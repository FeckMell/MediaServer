#include "stdafx.h"
#include "MSmgcp.h"
using namespace mgcp;


Control::Control()
{

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::PreprocessingINNER(string message_)
{
	SHP_IPL ipl = make_shared<IPL>(IPL(message_));
	LOG::Log(LOG::info, "MGCP", "MSMGCP: ipl:\n" + message_);

	if (ipl->data["From"] == "ann")
	{
		IN_ANN(ipl);
	}
	else
	{
		LOG::Log(LOG::fatal, "ERRORS", "In MGCP \"Control::PreprocessingINNER\" IPL error: not from ann. IPL=\n" + message_);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::PreprocessingOUTTER()
{
	SHP_MGCP mgcp = make_shared<MGCP>(MGCP(true));
	LOG::Log(LOG::info, "MGCP", "MSMGCP: message Parsed as:\n" + mgcp->PrintAll());
	if (mgcp->outerError == "")
	{
		string cmd = mgcp->data["CMD"];
		string type = mgcp->data["EventType"];
		if (cmd == "CRCX" && type == "ann")
		{
			CRCX_ANN(mgcp);
		}
		else if (cmd == "CRCX" && type == "cnf")
		{
			CRCX_CNF(mgcp);
		}
		else if (cmd == "RQNT" && type == "ann")
		{
			RQNT_ANN(mgcp);
		}
		else if (cmd == "MDCX" && type == "cnf")
		{
			MDCX_CNF(mgcp);
		}
		else if (cmd == "DLCX" && type == "ann")
		{
			DLCX_ANN(mgcp);
		}
		else if (cmd == "DLCX" && type == "cnf")
		{
			DLCX_CNF(mgcp);
		}
		else
		{
			mgcp->innerError = "Control::Preprocessing ERROR";
		}
	}
	mgcp->ReplyClient();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::CRCX_CNF(SHP_MGCP mgcp_)
{
	if (POINTSTORE::GetPoint(mgcp_->data["CallID"]) != nullptr)
	{
		mgcp_->innerError = "Control::CRCX_CNF WTF? ERROR";
		return;
	}
	if (mgcp_->data["EventID"] != "$" && FindCnf(mgcp_->data["EventID"]) == nullptr)
	{
		mgcp_->innerError = "Control::CRCX_CNF WTF?2 ERROR";
		return;
	}

	mgcp_->serverSDP.reset(new SDP(SOCKSTORE::ReservePort(), mgcp_->data["CallID"]));
	SHP_Point new_point = POINTSTORE::CreatePoint(mgcp_->data["CallID"], mgcp_->serverSDP);

	if (mgcp_->data["EventID"] == "$")
	{
		mgcp_->data["EventID"] = EVENTID::ReserveEventID();
		SHP_Cnf new_cnf = make_shared<Cnf>(Cnf(new_point, mgcp_->data["EventID"], mgcp_->clientSDP));
		vecCnfs.push_back(new_cnf);
	}
	else
	{
		SHP_Cnf found_cnf = FindCnf(mgcp_->data["EventID"]);
		found_cnf->AddPoint(new_point);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::CRCX_ANN(SHP_MGCP mgcp_)
{
	if (POINTSTORE::GetPoint(mgcp_->data["CallID"]) != nullptr)
	{
		mgcp_->innerError = "Control::CRCX_ANN WTF? ERROR";
		return;
	}
	if (mgcp_->data["EventID"] != "$")
	{
		mgcp_->innerError = "Control::CRCX_ANN WTF?2 ERROR";
		return;
	}
	mgcp_->data["EventID"] = EVENTID::ReserveEventID();

	mgcp_->serverSDP.reset(new SDP(SOCKSTORE::ReservePort(), mgcp_->data["CallID"]));
	SHP_Point new_point = POINTSTORE::CreatePoint(mgcp_->data["CallID"], mgcp_->serverSDP);

	SHP_Ann new_ann = make_shared<Ann>(Ann(new_point, mgcp_));
	vecAnns.push_back(new_ann);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::MDCX_CNF(SHP_MGCP mgcp_)
{
	SHP_Point found_point = POINTSTORE::GetPoint(mgcp_->data["CallID"]);
	SHP_Cnf found_cnf = FindCnf(mgcp_->data["EventID"]);
	if (found_point == nullptr || found_cnf == nullptr)
	{
		mgcp_->innerError = "Control::MDCX_CNF not found ERROR";
		return;
	}

	found_point->ModifyClientSDP(mgcp_->clientSDP);
	mgcp_->serverSDP = found_point->GetSDP("server");
	found_cnf->Process();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::RQNT_ANN(SHP_MGCP mgcp_)
{
	SHP_Point found_point = POINTSTORE::GetPoint(mgcp_->data["CallID"]);
	SHP_Ann found_ann = FindAnn(mgcp_->data["EventID"]);
	if (found_point == nullptr || found_ann == nullptr)
	{
		mgcp_->innerError = "Control::RQNT_ANN point\ann not found ERROR";
		return;
	}
	found_ann->RequestMusic(mgcp_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DLCX_CNF(SHP_MGCP mgcp_)
{
	SHP_Point found_point = POINTSTORE::GetPoint(mgcp_->data["CallID"]);
	SHP_Cnf found_cnf = FindCnf(mgcp_->data["EventID"]);
	if (found_point == nullptr || found_cnf == nullptr)
	{
		mgcp_->innerError = "Control::DLCX_CNF not found ERROR";
		return;
	}

	if (found_cnf->DeletePoint(found_point) == true){ RemoveCnf(found_cnf); }
	else{ found_cnf->Process(); }

	POINTSTORE::DeletePoint(mgcp_->data["CallID"]);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DLCX_ANN(SHP_MGCP mgcp_)
{
	SHP_Point found_point = POINTSTORE::GetPoint(mgcp_->data["CallID"]);
	SHP_Ann found_ann = FindAnn(mgcp_->data["EventID"]);
	if (found_point == nullptr || found_ann == nullptr)
	{
		mgcp_->innerError = "Control::DLCX_ANN point\ann not found ERROR";
		return;
	}
	found_ann->Stop();
	RemoveAnn(found_ann);
	POINTSTORE::DeletePoint(mgcp_->data["CallID"]);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::IN_ANN(SHP_IPL ipl_)
{
	SHP_Point found_point = POINTSTORE::GetPoint(ipl_->data["CallID"]);
	SHP_Ann found_ann = FindAnn(ipl_->data["EventID"]);
	if (ipl_->data["EventType"] == "LoopEnd")
	{
		found_ann->Stop();
		found_ann->SendNTFY();
	}
	else
	{
		LOG::Log(LOG::fatal, "ERRORS", "MSMGCP: IN_ANN unknown param:" + ipl_->data["EventType"] + "\n" + ipl_->ipl);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Cnf Control::FindCnf(string event_id_)
{
	for (auto &cnf : vecCnfs) if (cnf->eventID == event_id_) return cnf;
	return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::RemoveCnf(SHP_Cnf cnf_)
{
	vecCnfs.erase(std::remove(vecCnfs.begin(), vecCnfs.end(), cnf_), vecCnfs.end());
	EVENTID::FreeEventID(cnf_->eventID);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Ann Control::FindAnn(string event_id_)
{
	for (auto &ann : vecAnns) if (ann->eventID == event_id_) return ann;
	return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::RemoveAnn(SHP_Ann ann_)
{
	vecAnns.erase(std::remove(vecAnns.begin(), vecAnns.end(), ann_), vecAnns.end());
	EVENTID::FreeEventID(ann_->eventID);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------