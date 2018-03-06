#include "stdafx.h"
#include "SL_SDP.h"

int SDP::sdpID = 0;

SDP::SDP(string sdp_) :sdp(sdp_)
{
	Parse();
	Check();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SDP::SDP(string port_, string call_id_)
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
	sdp = str(template_sdp
		%CFG::data["outerIP"]
		% port_
		%sdpID
		%call_id_
		);
	sdpID++;
	Parse();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SDP::Parse()
{
	data.insert({ "IP", get_substr(sdp, "c=IN IP4 ", "\n") });
	data.insert({ "Port", get_substr(sdp, "m=audio ", " ") });
	data.insert({ "CallID", get_substr(sdp, "s=", "\n") });

	vector<string> alphabet = { "sendrecv", "inactive" };
	for (auto& e : alphabet)
	{
		if (sdp.find("a=" + e) != string::npos)
		{
			data.insert({ "Mode", e });
			break;
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SDP::Check()
{
	/*Check params existance*/
	vector<string> alphabet = { "IP", "Port", "Mode", "CallID" };
	for (auto& e : alphabet)
	{
		if (data[e] == "")
		{
			error += "BAD SDP " + e;
			return;
		}
	}

	/*Check if Mode is correct*/
	alphabet = { "sendrecv", "inactive" };
	error += "BAD SDP MODE";
	for (auto& e : alphabet)
	{
		if (data["Mode"] == e)
		{
			error = "";
			break;
		}
	}
	if (error != "") return;

	/*Check SDP Port*/
	try
	{
		int port = stoi(data["Port"]);
		if (port < 1 || port>65535)
		{
			error += "BAD SDP PORT 1";
			return;
		}
	}
	catch (exception& e)
	{
		error += "BAD SDP PORT 2";
		e;
		return;
	}

	/*Check SDP IP*/
	regex e(R"((\d+).(\d+).(\d+).(\d+))");
	cmatch result;
	try{ regex_match(data["IP"].c_str(), result, e); }
	catch (exception& e)
	{
		error += "BAD SDP IP 1";
		e;
		return;
	}
	if (result.size() != 5)
	{
		error += "BAD SDP IP 2";
		return;
	}
	for (int i = 1; i < (int)result.size(); ++i)
	{
		if (stoi(result.str(i)) < 0 || stoi(result.str(i)) > 255)
		{
			error += "BAD SDP IP 3";
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
	sdp = str(template_sdp
		%data["IP"]
		% data["Port"]
		% sdpID
		%data["CallID"]
		% mode_
		);
	data["Mode"] = mode_;
	sdpID++;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SDP SDP::Generate(string port_, string call_id_)
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
		));
	string result = str(template_sdp
		%CFG::data["outerIP"]
		% port_
		%sdpID
		%call_id_
		);
	sdpID++;
	return SDP(result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------