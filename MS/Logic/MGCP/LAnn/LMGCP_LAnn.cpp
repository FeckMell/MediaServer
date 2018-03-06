#include "stdafx.h"
#include "LMGCP_LAnn.h"
using namespace NLmgcpAnn;
map<string, SHP_Point> LAnn::mapPoint = {};
vector<int> LAnn::vecEventID = {};

void LAnn::Init(map<string, boost::function<void(boost::any)>>& map_func_)
{
	LOG::AddLogInstance("MGCPLAnn");
	map_func_["ann"] = boost::bind(&LAnn::Proceed, _1);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LAnn::Proceed(boost::any mess_)
{
	try
	{
		SHP_IPL ipl = boost::any_cast<SHP_IPL>(mess_);
		ProceedIPL(ipl);
	}
	catch (...)
	{
		SHP_MGCP mgcp = boost::any_cast<SHP_MGCP>(mess_);
		ProceedMGCP(mgcp);
		return;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LAnn::ProceedMGCP(SHP_MGCP mgcp_)
{
	if (mgcp_->Param("CMD") == "CRCX")
	{
		CRCX(mgcp_);
	}
	else if (mgcp_->Param("CMD") == "RQNT")
	{
		RQNT(mgcp_);
	}
	else if (mgcp_->Param("CMD") == "DLCX")
	{
		DLCX(mgcp_);
	}
	else
	{
		//2TODO
		cout << "LAnn unknown cmd";
		LOG::Log("fatal", "Errors", "LAnn unknown cmd:" + mgcp_->Param("CMD"));
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LAnn::ProceedIPL(SHP_IPL ipl_)
{
	if (ipl_->Param("CMD") == "DL")
	{
		SHP_Point found_point = mapPoint[ipl_->Param("CallID")];
		if (found_point == nullptr) return;
		found_point->DLCX();
		mapPoint.erase(found_point->Param("CallID"));
		FreeEventID(found_point->Param("EventID"));
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LAnn::CRCX(SHP_MGCP mgcp_)
{
	SHP_Point new_point = mapPoint[mgcp_->Param("CallID")];
	if (new_point != nullptr)
	{
		FreeEventID(new_point->Param("EventID"));
		new_point.reset();
	}
	mgcp_->SetParam("EventID", ReserveEventID());
	new_point.reset(new Point(mgcp_));
	mapPoint[mgcp_->Param("CallID")] = new_point;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LAnn::RQNT(SHP_MGCP mgcp_)
{
	SHP_Point found_point = mapPoint[mgcp_->Param("CallID")];
	if (found_point == nullptr)
	{
		mgcp_->SetParam("Error", "Not Found");
		return;
	}
	found_point->RQNT(mgcp_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LAnn::DLCX(SHP_MGCP mgcp_)
{
	SHP_Point found_point = mapPoint[mgcp_->Param("CallID")];
	if (found_point == nullptr)
	{
		mgcp_->SetParam("Error", "Not Found");
		return;
	}
	found_point->DLCX();
	mapPoint.erase(found_point->Param("CallID"));
	FreeEventID(found_point->Param("EventID"));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string LAnn::ReserveEventID()
{
	int free_event_id = 0;
	if (vecEventID.size() == 0) { vecEventID.push_back(free_event_id);  return to_string(free_event_id); }
	for (unsigned i = 0; i <vecEventID.size(); ++i)
	{
		if (vecEventID[i] != free_event_id)
		{
			vecEventID.push_back(free_event_id);
			sort(vecEventID.begin(), vecEventID.end());
			return to_string(free_event_id);
		}
		free_event_id++;
	}
	vecEventID.push_back(free_event_id);
	sort(vecEventID.begin(), vecEventID.end());
	return to_string(free_event_id);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LAnn::FreeEventID(string event_id_)
{
	vecEventID.erase(remove(vecEventID.begin(), vecEventID.end(), stoi(event_id_)), vecEventID.end());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------