#include "stdafx.h"
#include "MSmgcp.h"
using namespace mgcp;


Control::Control()
{
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::PreprocessingIN(string message_)
{
	cout << "\nvoid mgcp::Control::PreprocessingIN(string message_)";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::PreprocessingOUT(REQUEST message_)
{
	SHP_MGCP mgcp = make_shared<MGCP>(MGCP(message_.data, message_.sender));
	//LOG::Log(LOG::info, "MGCP", "MSMGCP: message is:\n" + message_.data);
	LOG::Log(LOG::info, "MGCP", "MSMGCP: message Paresed as:\n" + mgcp->PrintAll());
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
	if (FindPoint(mgcp_->data["CallID"]) != nullptr)
	{
		mgcp_->innerError = "Control::CRCX_CNF WTF? ERROR";
		return;
	}
	if (mgcp_->data["EventID"] != "$" && FindCnf(mgcp_->data["EventID"]) == nullptr)
	{
		mgcp_->innerError = "Control::CRCX_CNF WTF?2 ERROR";
		return;
	}

	mgcp_->serverSDP.reset(new SDP(SSTORAGE::ReservePort(), mgcp_->data["CallID"]));
	SHP_Point new_point = make_shared<Point>(Point(mgcp_));
	vecPoints.push_back(new_point);

	if (mgcp_->data["EventID"] == "$")
	{
		mgcp_->data["EventID"] = SSTORAGE::ReserveEventID();
		SHP_Cnf new_cnf = make_shared<Cnf>(Cnf(new_point, mgcp_->data["EventID"]));
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
	if (FindPoint(mgcp_->data["CallID"]) != nullptr)
	{
		mgcp_->innerError = "Control::CRCX_ANN WTF? ERROR";
		return;
	}
	if (mgcp_->data["EventID"] != "$")
	{
		mgcp_->innerError = "Control::CRCX_ANN WTF?2 ERROR";
		return;
	}
	mgcp_->data["EventID"] = SSTORAGE::ReserveEventID();

	mgcp_->serverSDP.reset(new SDP(SSTORAGE::ReservePort(), mgcp_->data["CallID"]));
	SHP_Point new_point = make_shared<Point>(Point(mgcp_));
	vecPoints.push_back(new_point);

	SHP_Ann new_ann = make_shared<Ann>(Ann(new_point, mgcp_));
	vecAnns.push_back(new_ann);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::MDCX_CNF(SHP_MGCP mgcp_)
{
	SHP_Point found_point = FindPoint(mgcp_->data["CallID"]);
	SHP_Cnf found_cnf = FindCnf(mgcp_->data["EventID"]);
	if (found_point == nullptr || found_cnf == nullptr)
	{
		mgcp_->innerError = "Control::MDCX_CNF not found ERROR";
		return;
	}
	
	found_point->ModifyPoint(mgcp_);
	found_cnf->Process();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::RQNT_ANN(SHP_MGCP mgcp_)
{
	SHP_Point found_point = FindPoint(mgcp_->data["CallID"]);
	SHP_Ann found_ann = FindAnn(mgcp_->data["EventID"]);
	if (found_point == nullptr)
	{
		mgcp_->innerError = "Control::RQNT_ANN point not found ERROR";
		return;
	}
	if (found_ann == nullptr)
	{
		mgcp_->innerError = "Control::RQNT_ANN ann not found ERROR";
		return;
	}
	found_ann->RequestMusic(mgcp_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DLCX_CNF(SHP_MGCP mgcp_)
{
	SHP_Point found_point = FindPoint(mgcp_->data["CallID"]);
	SHP_Cnf found_cnf = FindCnf(mgcp_->data["EventID"]);
	if (found_point == nullptr || found_cnf == nullptr)
	{
		mgcp_->innerError = "Control::DLCX_CNF not found ERROR";
		return;
	}

	if (found_cnf->DeletePoint(found_point) == true){ RemoveCnf(found_cnf); }
	else{ found_cnf->Process(); }

	RemovePoint(found_point);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DLCX_ANN(SHP_MGCP mgcp_)
{
	SHP_Point found_point = FindPoint(mgcp_->data["CallID"]);
	SHP_Ann found_ann = FindAnn(mgcp_->data["EventID"]);
	if (found_point == nullptr)
	{
		mgcp_->innerError = "Control::DLCX_ANN point not found ERROR";
		return;
	}
	if (found_ann == nullptr)
	{
		mgcp_->innerError = "Control::DLCX_ANN ann not found ERROR";
		return;
	}
	found_ann->Delete();
	RemoveAnn(found_ann);
	RemovePoint(found_point);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Point Control::FindPoint(string call_id_)
{
	for (auto &point : vecPoints) if (point->callID == call_id_) return point;
	return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::RemovePoint(SHP_Point point_)
{
	vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
	SSTORAGE::FreePort(point_->serverSDP->data["Port"]);
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
	SSTORAGE::FreeEventID(cnf_->eventID);
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
	SSTORAGE::FreeEventID(ann_->eventID);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------