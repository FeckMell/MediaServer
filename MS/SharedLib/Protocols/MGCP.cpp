#include "stdafx.h"
#include "MGCP.h"

MGCP::MGCP(string request_, EP endpoint_) :endPoint(endpoint_)
{
	//clientSDP.reset(new SDP());
	//serverSDP.reset(new SDP());
	mapData["Request"] = request_;
	Remove();
	SplitMGCPandSDP();
	ParseMain();
	ParseRest();
	CheckValid();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::Remove()
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
	int line = 0;
	string temp = copy_single_n_line(Param("Request"), line);
	while (temp != "")
	{
		mapData["MGCP"] += temp + "\n";
		line++;
		temp = copy_single_n_line(Param("Request"), line);
	}
	line++;
	string sdp = "";
	temp = copy_single_n_line(Param("Request"), line);
	while (temp != "")
	{
		sdp += temp + "\n";
		line++;
		temp = copy_single_n_line(Param("Request"), line);
	}
	if (sdp != "") clientSDP.reset(new SDP(sdp));
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
	catch (exception& e)
	{
		mapData["Error"] = "Not MGCP";
		e;
		return;
	}
	if (result.size() != 6)
	{
		mapData["Error"] = "Not MGCP";
		return;
	}
	mapData["CMD"] = result.str(1);
	mapData["MessNum"] = result.str(2);
	mapData["EventType"] = result.str(3);
	mapData["EventID"] = result.str(4);
	mapData["Addr"] = result.str(5);
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
		mapData[result.str(1)] = result.str(2);
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
		mapData["Error"] = "NOT MGCP";
		return;
	}

	// Check if EventType is valid
	if (!(Param("EventType") == "ann" || Param("EventType") == "cnf"))
	{
		mapData["Error"] = "NOT MGCP";
		return;
	}

	// Check if EventID is valid
	if (Param("EventID") != "$")
	{
		try
		{
			stoi(Param("EventID"));
		}
		catch (exception& e)
		{
			mapData["Error"] = "NOT MGCP";
			e;
			return;
		}
	}

	// Check if CallID is valid and change key value
	if (Param("C") == "")
	{
		if (Param("X") == "")
		{
			mapData["Error"] = "NOT MGCP";
			return;
		}
		else
		{
			mapData["CallID"] = Param("X");
			//mapData.erase("X"); DONOT DELETE
			//mapData.erase("C"); DONOT DELETE
		}
	}
	else
	{
		mapData["CallID"] = Param("C");
		//data.erase("C"); DONOT DELETE
	}

	// Check if MessNum is valid
	try
	{
		stoi(Param("MessNum"));
	}
	catch (exception& e)
	{
		mapData["Error"] = "NOT MGCP";
		e;
		return;
	}

	//Check ann param Q
	if (Param("CMD") == "RQNT" && Param("EventType") == "ann")
	{
		if (!(Param("Q") == "loop" || Param("Q") == "once"))
		{
			mapData["Error"] = "BAD param Q";
			return;
		}
	}

	// Check SDP
	if (clientSDP != nullptr)
	{
		if (clientSDP->Param("Error") != "")
		{
			mapData["Error"] = clientSDP->Param("Error");
			return;
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::ReplyClient()
{
	if (Param("Error") != "") SendClient("400 999 BAD\nZ: outerError " + Param("Error") + "\nRequestWas:\n" + Param("Request")); // Not MGCP
	else SendClient(ResponseOK());
}
string MGCP::ReplyClient2()
{
	if (Param("Error") != "") return ("400 999 BAD\nZ: outerError " + Param("Error") + "\nRequestWas:\n" + Param("Request")); // Not MGCP
	else return (ResponseOK());
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

		if (serverSDP!= nullptr/*->Param("SDP") != ""*/) result += "\n\n" + serverSDP->Param("SDP");
	}
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MGCP::PrintAll()
{
	string result = "";
	for (auto& e : mapData) result += "\n_" + e.first + "_=_" + e.second + "_";
	if (clientSDP != nullptr) result += "\nSDP:\n" + clientSDP->Param("SDP");
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MGCP::Param(string name_)
{
	return mapData[name_];
}
SHP_SDP MGCP::GetSDP(string type_)
{
	if (type_ == "Server") return serverSDP;
	else if (type_ == "Client") return clientSDP;
	else
	{
		cout << "\nBad: MGCP::GetSDP(string type_)";
		system("pause");
		exit(-1);
	}
}
void MGCP::SetSDP(string type_, SHP_SDP new_sdp_)
{
	if (type_ == "Server") serverSDP = new_sdp_;
	else if (type_ == "Client") clientSDP = new_sdp_;
	else
	{
		//2TODO
		cout << "\n BAD MGCP::SetSDP(string type_, SHP_SDP new_sdp_)";
		system("pause");
		exit(-1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::SetParam(string name_, string value_)
{
	mapData[name_] = value_;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::SendClient(string str_)
{
	//LOG::Log(LOG::info, "MGCP", "MSMGCP: MGCP sent client:\n" + str_);
	//NET::GS(NET::OUTER::mgcp_)->s.send_to(boost::asio::buffer(str_), endPoint);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::ReplyNTFY()
{
	string result = "NTFY " + mapData["MessNum"] + " " + mapData["EventType"] + "/" + mapData["EventID"] + "@[" + mapData["Addr"] + "]" + "mgcp 1.0";
	result += "\nC: " + mapData["CallID"];
	SendClient(result);
}
string MGCP::ReplyNTFY2()
{
	string result = "NTFY " + mapData["MessNum"] + " " + mapData["EventType"] + "/" + mapData["EventID"] + "@[" + mapData["Addr"] + "]" + "mgcp 1.0";
	result += "\nC: " + mapData["CallID"];
	return result;
}