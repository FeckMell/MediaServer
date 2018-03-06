#include "stdafx.h"
#include "LMGCP_LAnnPoint.h"
using namespace NLmgcpAnn;

Point::Point(SHP_MGCP mgcp_)
{
	mapData["EventID"] = mgcp_->Param("EventID");
	CallerStore::CreateCaller(mgcp_->Param("CallID"));
	baseCaller = CallerStore::GetCaller(mgcp_->Param("CallID"));
	baseCaller->SetParam("Creator", "mgcp");
	baseCaller->SetParam("Type", "ann");
	mgcp_->SetSDP("Server", baseCaller->GetSDP("Server"));
	baseCaller->SetSDP("Client", mgcp_->GetSDP("Client"));
}
Point::~Point()
{
	CallerStore::DelCaller(Param("CallID"));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::RQNT(SHP_MGCP mgcp_)
{
	mapData["FileName"] = get_substr(mgcp_->Param("S"), ",file:///", ")");
	if (!CheckFileExistance())
	{
		mgcp_->SetParam("Error", "File=" + Param("FileName") + " does not exist");
		LOG::Log("fatal", "Errors", "NLmgcpAnn::Point::RQNT File=" + Param("FileName") + " does not exist");
		return;
	}
	mapData["Loop"] = mgcp_->Param("Q");
	if (baseCaller->Check() && Param("State") == "")
	{
		mapData["State"] = "SendingAnn";
		SendModul("CR");
	}
	else
	{
		LOG::Log("fatal", "Errors", "NLmgcpAnn::Point::RQNT BadCheck CallID:"+Param("CallID"));
		mapData["Error"] = "Bad: baseCaller->Check() && mapData[\"State\"]==\"\"";
		cout << Param("Error");
		//system("pause");
		//exit(-1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::DLCX()
{
	if (mapData["State"] == "SendingAnn")
	{
		mapData["State"] = "";
		SendModul("DL");
	}
	else
	{
		//2TODO
		LOG::Log("fatal", "Errors", "NLmgcpAnn::Point::DLCX BadCheck CallID:" + Param("CallID"));
		mapData["Error"] = "Point::DLCX()";
		cout << Param("Error");
		//system("pause");
		//exit(-1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Point::Param(string name_)
{
	if (name_ == "State") return mapData["State"];
	else if (name_ == "Loop") return mapData["Loop"];
	else if (name_ == "EventID") return mapData["EventID"];
	else if (name_ == "Error") return mapData["Error"];
	else if (name_ == "FileName") return mapData["FileName"];
	else { return baseCaller->Param(name_); }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Point::CheckFileExistance()
{
	ifstream file(CFG::Param("MediaPath") + CFG::Param("Slash") + Param("FileName"));
	if (file.is_open()) { file.close(); return true; }
	else{ return false; }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::SendModul(string type_)
{
	string result = "IPL\n";
	result += "From=mgcp\n";
	result += "To=ann\n";
	result += "CMD=" + type_ + "\n";
	result += "CallID=" + Param("CallID") + "\n";
	result += "FileName=" + Param("FileName") + "\n";
	result += "Loop=" + Param("Loop") + "\n";

	COM::SendModul("ann", result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------