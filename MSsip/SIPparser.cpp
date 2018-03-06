#include "stdafx.h"
#include "SIPparser.h"


SIP::SIP(char* rawMes_, EP sender_) : request(rawMes_), sender(sender_)
{
	Remove();
	SplitSIPandSDP();
	data.resize(maxqqq);
	ParseMain();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIP::SplitSIPandSDP()
{
	int line = 0;
	string temp = copy_single_n_line(request, line);
	while (temp != "")
	{
		sip += temp + "\n";
		line++;
		temp = copy_single_n_line(request, line);
	}
	line++;
	temp = copy_single_n_line(request, line);
	while (temp != "")
	{
		sdp += temp + "\n";
		line++;
		temp = copy_single_n_line(request, line);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIP::Remove()
{
	auto fd_pos = request.find("\r");
	while (fd_pos != string::npos)
	{
		request.erase(request.begin() + fd_pos);
		fd_pos = request.find("\r", fd_pos - 1);
	}
	auto fd_pos2 = request.find("  ");
	while (fd_pos2 != string::npos)
	{
		request.erase(request.begin() + fd_pos2);
		fd_pos2 = request.find("  ", fd_pos2 - 1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIP::ParseMain()
{
	data[CMD] = sip.substr(0, copy_single_n_line(sip, 0).find(" sip:"));
	data[CallID] = get_substr(sip, "Call-ID: ", "\n");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string SIP::GetParam(int c_)
{
	switch (c_)
	{
	case CMD: return data[CMD];
	case CallID: return data[CallID];
	default: assert(false); return "";
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string SIP::ReplyRinging()
{
	string result = "SIP/2.0 180 Ringing\n";
	int line = 0;
	int temp_value = 0;
	string temp_line = copy_single_n_line(sip, line);
	while (temp_line != "")
	{
		if (temp_line.find("Via: ") != string::npos)
		{
			if (temp_value == 0)
			{
				result += temp_line + ";received=" + sender.address().to_string() + "\n";
				temp_value++;
			}
			else
			{
				result += temp_line + "\n";
			}
		}
		else if (temp_line.find("To: ") != string::npos)
		{
			result += temp_line + ";tag=tag_value\n";
		}
		else if (temp_line.find("From: ") != string::npos)
		{
			result += temp_line + "\n";
		}
		else if (temp_line.find("Call-ID: ") != string::npos)
		{
			result += temp_line + "\n";
		}
		else if (temp_line.find("CSeq: ") != string::npos)
		{
			result += temp_line + "\n";
		}
		line++;
		temp_line = copy_single_n_line(sip, line);
	}
	result += "Contact: My name+IP\n";
	result += "Content-Length: 0\n";
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string SIP::ReplyOK(string sdp_)
{
	string result = "SIP/2.0 200 OK\n";
	int line = 0;
	int temp_value = 0;
	string temp_line = copy_single_n_line(sip, line);
	while (temp_line != "")
	{
		if (temp_line.find("Via: ") != string::npos)
		{
			if (temp_value == 0)
			{
				result += temp_line + ";received=" + sender.address().to_string() + "\n";
				temp_value++;
			}
			else
			{
				result += temp_line + "\n";
			}
		}
		else if (temp_line.find("To: ") != string::npos)
		{
			result += temp_line + ";tag=tag_value\n";
		}
		else if (temp_line.find("From: ") != string::npos)
		{
			result += temp_line + "\n";
		}
		else if (temp_line.find("Call-ID: ") != string::npos)
		{
			result += temp_line + "\n";
		}
		else if (temp_line.find("CSeq: ") != string::npos)
		{
			result += temp_line + "\n";
		}
		line++;
		temp_line = copy_single_n_line(sip, line);
	}
	result += "Contact: My name+IP\n";
	if (sdp_ != "")
	{
		result += "Content-Type: application/sdp\n";
		result += "Content-Length: " + to_string(sdp_.length()) + "\n";
		result += "\n" + sdp_;
	}
	else
	{
		result += "Content-Length: 0\n";
	}
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string SIP::ResponseBAD()
{
	return "SIP/2.0 400 Bad Request\n";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIP::ReplyClient(SHP_SOCK s_, string str_)
{
	BOOST_LOG_SEV(lg, warning) << "Reply is:\n" << str_;
	s_->s.send_to(boost::asio::buffer(str_), sender);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------