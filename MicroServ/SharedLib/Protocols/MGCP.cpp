#include "stdafx.h"
#include "MGCP.h"

MGCP::MGCP(SHP_SockBuf request_, EP ep_) : endPoint(ep_)
{
	mapSDP.insert({ "Client", nullptr });
	mapSDP.insert({ "Server", nullptr });
	SetParam("Reauest", string((char*)request_->Data()));
	SetParam("RequestSize", to_string(request_->Size()));

	RemoveExtra();
	SplitMGCPandSDP();
	ParseMain();
	ParseRest();
	CheckValid();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
MGCP::MGCP()
{
	mapSDP.insert({ "Client", nullptr });
	mapSDP.insert({ "Server", nullptr });
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_SDP MGCP::GetSDP(string type_)
{
	return mapSDP[type_];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::SetSDP(string type_, SHP_SDP new_sdp_)
{
	mapSDP[type_] = new_sdp_;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MGCP::Param(string name_)
{
	if (name_ == "RequestIP") return endPoint.address().to_string();
	else if (name_ == "RequestPort") return to_string(endPoint.port());

	return mapData[name_];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::SetParam(string name_, string value_)
{
	mapData[name_] = value_;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MGCP::PrintAll()
{
	string result = "MGCP DATA:";
	result += "\nGot request from:" + endPoint.address().to_string() + ":" + to_string(endPoint.port());
	for (auto& e: mapData) result += "\n_" + e.first + "_=_" + e.second + "_";
	for (auto& e : mapSDP) if (e.second != nullptr) result += "\n\nthis MGCP contains SDP "+ e.first+":" + e.second->PrintAll();
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::RemoveExtra()
{
	auto fd_pos = mapData["Request"].find("\r");
	while (fd_pos != string::npos)
	{
		mapData["Request"].erase(mapData["Request"].begin() + fd_pos);
		fd_pos = mapData["Request"].find("\r");
	}
	auto fd_pos2 = mapData["Request"].find("  ");
	while (fd_pos2 != string::npos)
	{
		mapData["Request"].erase(mapData["Request"].begin() + fd_pos2);
		fd_pos2 = mapData["Request"].find("  ", fd_pos2 - 1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::SplitMGCPandSDP()
{
	string temp_mgcp = "";
	int line = 0;
	string temp = copy_single_n_line(Param("Request"), line);
	while (temp != "")
	{
		temp_mgcp += temp + "\n";
		line++;
		temp = copy_single_n_line(Param("Request"), line);
	}
	SetParam("MGCP", temp_mgcp);
	line++;

	string temp_sdp = "";
	temp = copy_single_n_line(Param("Request"), line);
	while (temp != "")
	{
		temp_sdp += temp + "\n";
		line++;
		temp = copy_single_n_line(Param("Request"), line);
	}
	if (temp_sdp != "") mapSDP["Client"].reset(new SDP(temp_sdp));
	SetParam("SDP", temp_sdp);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::ParseMain()
{
	string line = copy_single_n_line(Param("MGCP"), 0);
	cout << "\n\nLINE=_" << line << "_";
	regex e(R"((\w{4}) (\d+) (\w{3})\/(.+)@\[(.+)\] \w{4} 1.0 \w{3} 1.0)");
	cmatch result;
	try{ regex_match(line.c_str(), result, e); }
	catch (...)
	{
		SetParam("Error", "Not MGCP");
		return;
	}
	if (result.size() != 6)
	{
		SetParam("Error", "Not MGCP");
		return;
	}
	SetParam("CMD",result.str(1));
	SetParam("MessNum",result.str(2));
	SetParam("EventType",result.str(3));
	SetParam("EventID",result.str(4));
	SetParam("Addr",result.str(5));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::ParseRest()
{
	int line_num = 1;
	regex e(R"((\w): (.+))");
	string line = copy_single_n_line(Param("MGCP"), line_num);
	while (line != "")
	{
		cmatch result;
		regex_match(line.c_str(), result, e);
		SetParam(result.str(1), result.str(2));
		line_num++;
		line = copy_single_n_line(Param("MGCP"), line_num);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::CheckValid()
{
	// Check if CMD is valid
	if (!(Param("CMD") == "CRCX" || Param("CMD") == "MDCX" || Param("CMD") == "DLCX" || Param("CMD") == "RQNT"))
	{
		SetParam("Error", "NOT MGCP CMD");
		return;
	}


	// Check if EventType is valid
	if (!(Param("EventType") == "ann" || Param("EventType") == "cnf"))
	{
		SetParam("Error", "NOT MGCP EventType");
		return;
	}


	// Check if EventID is valid
	if (Param("EventID") != "$")
	{
		try { stoi(Param("EventID")); }
		catch (...)
		{
			SetParam("Error", "NOT MGCP EventID");
			return;
		}
	}


	// Check if CallID is valid and change key value
	if (Param("C") == "")
	{
		if (Param("X") == "")
		{
			SetParam("Error", "NOT MGCP CallID");
			return;
		}
		else{ SetParam("CallID", Param("X")); }
	}
	else{ SetParam("CallID", Param("C")); }


	// Check if MessNum is valid
	try{ stoi(Param("MessNum")); }
	catch (...)
	{
		SetParam("Error", "NOT MGCP MessNum");
		return;
	}


	//Check ann param Q
	if (Param("CMD") == "RQNT" && Param("EventType") == "ann")
	{
		if (!(Param("Q") == "loop" || Param("Q") == "once"))
		{
			SetParam("Error", "NOT MGCP Q");
			return;
		}
	}


	// Check SDP
	if (mapSDP["Client"] != nullptr)
	{
		if (mapSDP["Client"]->Param("Error") != "")
		{
			SetParam("Error", mapSDP["Client"]->Param("Error"));
			return;
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MGCP::Reply()
{
	if (Param("Error") != "") return ("400 999 BAD\nZ: outerError " + Param("Error") + "\nRequestWas:\n" + Param("Request")); // Not MGCP
	else return (ResponseOK());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MGCP::ReplyNTFY()
{
	string result = "NTFY " + Param("MessNum") + " " + Param("EventType") + "/" + Param("EventID") + "@[" + Param("Addr") + "]" + "mgcp 1.0";
	result += "\nC: " + Param("CallID");
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MGCP::ResponseOK()
{
	string result;

	if (Param("CMD") == "DLCX") result = "250 " + Param("MessNum") + " OK";
	else  result = "200 " + Param("MessNum") + " OK";

	if (Param("CMD") == "CRCX" || Param("CMD") == "MDCX")
	{
		result += "\nZ: " + Param("EventType") + "/" + Param("EventID") + "@[" + Param("Addr") + "]";
		result += "\nI: " + to_string(rand() % 10000);

		if (GetSDP("Server") != nullptr) result += "\n\n" + GetSDP("Server")->Param("SDP");
	}
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------