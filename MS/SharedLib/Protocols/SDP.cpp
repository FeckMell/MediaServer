#include "stdafx.h"
#include "SDP.h"

int SDP::sdpID = 0;

SDP::SDP(string request_)
{
	mapData["SDP"] = request_;
	Parse();
	Check();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SDP::SDP(string call_id_, string s_port_)
{
	boost::format template_sdp = boost::format(string(
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
		));
	mapData["SDP"] = str(template_sdp
		%CFG::Param("OuterIP")
		% s_port_
		%sdpID
		%call_id_
		);
	mapData["CallID"] = call_id_;
	mapData["Port"] = s_port_;
	mapData["IP"] = CFG::Param("OuterIP");
	mapData["Mode"] = "sendrecv";
	sdpID++;
	Parse();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SDP::Parse()
{
	mapData.insert({ "IP", get_substr(Param("SDP"), "c=IN IP4 ", "\n") });
	mapData.insert({ "Port", get_substr(Param("SDP"), "m=audio ", " ") });
	mapData.insert({ "CallID", get_substr(Param("SDP"), "s=", "\n") });

	vector<string> alphabet = { "sendrecv", "inactive" };
	for (auto& e : alphabet)
	{
		if (Param("SDP").find("a=" + e) != string::npos)
		{
			mapData.insert({ "Mode", e });
			break;
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SDP::Check()
{
	vector<string> alphabet = { "IP", "Port", "Mode", "CallID" };
	for (auto& e : alphabet)
	{
		if (Param(e) == "")
		{
			mapData["Error"] += "BAD SDP " + e;
			return;
		}
	}

	try
	{
		int port = stoi(Param("Port"));
		if (port < 1 || port>65535)
		{
			mapData["Error"] += "BAD SDP PORT 1";
			return;
		}
	}
	catch (exception& e)
	{
		mapData["Error"] += "BAD SDP PORT 2";
		e;
		return;
	}

	regex e(R"((\d+).(\d+).(\d+).(\d+))");
	cmatch result;
	try{ regex_match(Param("IP").c_str(), result, e); }
	catch (exception& e)
	{
		mapData["Error"] += "BAD SDP IP 1";
		e;
		return;
	}
	if (result.size() != 5)
	{
		mapData["Error"] += "BAD SDP IP 2";
		return;
	}
	for (int i = 1; i < (int)result.size(); ++i)
	{
		if (stoi(result.str(i)) < 0 || stoi(result.str(i)) > 255)
		{
			mapData["Error"] += "BAD SDP IP 3";
			return;
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SDP::ChangeModeS(string mode_)
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
		"a=%5%\n"//5
		));
	mapData["SDP"] = str(template_sdp
		% Param("IP")
		% Param("Port")
		% sdpID
		% Param("CallID")
		% mode_
		);
	mapData["Mode"] = mode_;
	sdpID++;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string SDP::Param(string name_)
{
	return mapData[name_];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string PrintAll()
{
	//2TODO
	return "";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------