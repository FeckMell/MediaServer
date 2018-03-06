#include "stdafx.h"
#include "SL_SIP.h"

SIP::SIP(bool parse_) : sender(COM::GS(COM::OUTER::sip_)->endPoint)
{

	clientSDP.reset(new SDP());
	serverSDP.reset(new SDP());

	string request((char*)COM::GS(COM::OUTER::sip_)->buffer);
	Remove(request);
	SplitSIPandSDP(request);
	ParseMain();

	Check();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIP::SplitSIPandSDP(string& mes_)
{
	int line = 0;
	string temp = copy_single_n_line(mes_, line);
	while (temp != "")
	{
		sip += temp + "\n";
		line++;
		temp = copy_single_n_line(mes_, line);
	}
	line++;
	string sdp = "";
	temp = copy_single_n_line(mes_, line);
	while (temp != "")
	{
		sdp += temp + "\n";
		line++;
		temp = copy_single_n_line(mes_, line);
	}
	if (sdp != "") clientSDP.reset(new SDP(sdp));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIP::Remove(string& mes_)
{
	auto fd_pos = mes_.find("\r");
	while (fd_pos != string::npos)
	{
		mes_.erase(mes_.begin() + fd_pos);
		fd_pos = mes_.find("\r", fd_pos - 1);
	}
	auto fd_pos2 = mes_.find("  ");
	while (fd_pos2 != string::npos)
	{
		mes_.erase(mes_.begin() + fd_pos2);
		fd_pos2 = mes_.find("  ", fd_pos2 - 1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIP::ParseMain()
{
	data["CMD"] = sip.substr(0, copy_single_n_line(sip, 0).find(" sip:"));
	data["CallID"] = get_substr(sip, "Call-ID: ", "\n");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIP::Check()
{
	if (data["CMD"] == "") { outerError = "NOT SIP"; return; }
	if (!(data["CMD"] == "INVITE" || data["CMD"] == "BYE" || data["CMD"] == "ACK"))
	{
		outerError = "NOT SIP";
		return;
	}
	if (data["CallID"] == ""){ outerError = "NOT SIP"; return; }

	if (data["CMD"] == "INVITE" && clientSDP->sdp == "")
	{
		outerError = "NOT SUPPORTED SDP NEEDED";
		return;
	}
	if (clientSDP->error != "") { outerError = clientSDP->error; return; }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string SIP::ReplyRinging()
{
	string result = "SIP/2.0 180 Ringing\r\n";
	int line = 0;
	int temp_value = 0;
	string temp_line = copy_single_n_line(sip, line);
	while (temp_line != "")
	{
		if (temp_line.find("Via: ") != string::npos)
		{
			if (temp_value == 0)
			{
				result += temp_line + ";received=" + sender.address().to_string() + "\r\n";
				temp_value++;
			}
			else
			{
				result += temp_line + "\r\n";
			}
		}
		else if (temp_line.find("To: ") != string::npos)
		{
			result += temp_line + ";tag=MediaServerSIP\r\n";//TODO
		}
		else if (temp_line.find("From: ") != string::npos)
		{
			result += temp_line + "\r\n";
		}
		else if (temp_line.find("Call-ID: ") != string::npos)
		{
			result += temp_line + "\r\n";
		}
		else if (temp_line.find("CSeq: ") != string::npos)
		{
			result += temp_line + "\r\n";
		}
		line++;
		temp_line = copy_single_n_line(sip, line);
	}

	//result += "Contact: <sip:6801@10.77.7.5:5060>\r\n";//TODO
	result += "Contact: <sip:" + CFG::data["sipName"] + "@" + CFG::data["outerIP"] + ":" + CFG::data["sipPort"] + ">\r\n";
	result += "Content-Length: 0\r\n\r\n";
	return result;
	//SendClient(result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string SIP::ReplyOK()
{
	string result = "SIP/2.0 200 OK\r\n";
	int line = 0;
	int temp_value = 0;
	string temp_line = copy_single_n_line(sip, line);
	while (temp_line != "")
	{
		if (temp_line.find("Via: ") != string::npos)
		{
			if (temp_value == 0)
			{
				result += temp_line + ";received=" + sender.address().to_string() + "\r\n";
				temp_value++;
			}
			else
			{
				result += temp_line + "\r\n";
			}
		}
		else if (temp_line.find("To: ") != string::npos)
		{
			if (data["CMD"] == "INVITE") result += temp_line + ";tag=MediaServerSIP\r\n";
			else result += temp_line + "\r\n";
		}
		else if (temp_line.find("From: ") != string::npos)
		{
			result += temp_line + "\r\n";
		}
		else if (temp_line.find("Call-ID: ") != string::npos)
		{
			result += temp_line + "\r\n";
		}
		else if (temp_line.find("CSeq: ") != string::npos)
		{
			result += temp_line + "\r\n";
		}
		line++;
		temp_line = copy_single_n_line(sip, line);
	}
	result += "Contact: <sip:" + CFG::data["sipName"] + "@" + CFG::data["outerIP"] + ":" + CFG::data["sipPort"] + ">\r\n";
	if (serverSDP->sdp != "")
	{
		result += "Content-Type: application/sdp\r\n";
		result += "Content-Length: " + to_string(serverSDP->sdp.length()) + "\r\n";
		result += "\r\n" + serverSDP->sdp;
	}
	else
	{
		result += "Content-Length: 0\r\n\r\n";
	}
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIP::ReplyClient()
{
	if (innerError != "" || outerError != "")
	{
		SendClient("SIP/2.0 400 Bad Request\r\n" + innerError + " " + outerError + "\r\n");
		return;
	}
	else
	{
		if (data["CMD"] == "INVITE")
		{
			SendClient(ReplyRinging());
			SendClient(ReplyOK());
		}
		else
		{
			SendClient(ReplyOK());
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIP::SendClient(string str_)
{
	LOG::Log(LOG::info, "SIP", "MSSIP: SIP Reply:\n" + str_);
	COM::GS(COM::OUTER::sip_)->s.send_to(boost::asio::buffer(str_), sender);
}
