#include "stdafx.h"
#include "MGCPparser.h"
using namespace mgcp;


MGCP::MGCP(char* rawMes_, boost::asio::ip::udp::endpoint sender_) : request(rawMes_), sender(sender_)
{
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
	auto fd_pos = request.find("\r");
	while (fd_pos != string::npos)
	{
		request.erase(request.begin() + fd_pos);
		fd_pos = request.find("\r");
	}
	//auto fd_pos = mgcp.find("\r");
	//while (fd_pos != string::npos)
	//{
	//	mgcp.erase(mgcp.begin() + fd_pos);
	//	fd_pos = mgcp.find("\r", fd_pos - 1);
	//}
	auto fd_pos2 = request.find("  ");
	while (fd_pos2 != string::npos)
	{
		request.erase(request.begin() + fd_pos2);
		fd_pos2 = request.find("  ", fd_pos2 - 1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::SplitMGCPandSDP()
{
	int line = 0;
	string temp = copy_single_n_line(request, line);
	while (temp != "")
	{
		mgcp += temp + "\n";
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
void MGCP::ParseMain()
{
	string line = copy_single_n_line(mgcp, 0);
	cout << "\n\nLINE=_" << line << "_";
	regex e(R"((\w{4}) (\d+) (\w{3})\/(.+)@\[(.+)\] \w{4} 1.0 \w{3} 1.0)");
	cmatch result;
	try{ regex_match(line.c_str(), result, e); }
	catch (exception& e)
	{
		outerError = "Not MGCP";
		cout << e.what();
		return;
	}
	if (result.size() != 6)
	{
		outerError = "Not MGCP";
		return;
	}
	data.insert({ "CMD", result.str(1) });
	data.insert({ "MessNum", result.str(2) });
	data.insert({ "EventType", result.str(3) });
	data.insert({ "EventID", result.str(4) });
	data.insert({ "Addr", result.str(5) });
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::ParseRest()
{
	int line_num = 1;
	regex e(R"((\w): (.+))");
	string line = copy_single_n_line(mgcp, line_num);
	while (line != "")
	{
		cmatch result;
		regex_match(line.c_str(), result, e);
		data.insert({ result.str(1), result.str(2) });
		line_num++;
		line = copy_single_n_line(mgcp, line_num);
	}

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::CheckValid()
{
	// Check if CMD is valid
	if (!(data["CMD"] == "CRCX" || data["CMD"] == "MDCX" || data["CMD"] == "DLCX" || data["CMD"] == "RQNT"))
	{
		outerError = "NOT MGCP";
		return;
	}

	// Check if EventType is valid
	if (!(data["EventType"] == "ann" || data["EventType"] == "cnf"))
	{
		outerError = "NOT MGCP";
		return;
	}

	// Check if EventID is valid
	if (data["EventID"] != "$")
	{
		try
		{
			stoi(data["EventID"]);
		}
		catch (exception& e)
		{
			outerError = "NOT MGCP";
			e;
			return;
		}
	}

	// Check if CallID is valid and change key value
	if (data["C"] == "")
	{
		if (data["X"] == "")
		{
			outerError = "NOT MGCP";
			return;
		}
		else
		{
			data.insert({ "CallID", data["X"] });
			data.erase("X");
			data.erase("C");
		}
	}
	else
	{
		data.insert({ "CallID", data["C"] });
		data.erase("C");
	}

	// Check if MessNum is valid
	try
	{
		stoi(data["MessNum"]);
	}
	catch (exception& e)
	{
		outerError = "NOT MGCP";
		e;
		return;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::ReplyClient()
{
	if (outerError != "") ReplyNOTMGCP();
	else
	{
		string result;
		if (innerError != "") result = ResponseBAD();
		else result = ResponseOK();
		BOOST_LOG_SEV(LOG::vecLogs, info) << result;
		NET::GS(NET::mgcp)->s.send_to(boost::asio::buffer(result), sender);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MGCP::ResponseOK()
{
	string result;

	if (data["CMD"] == "DLCX") result = "250 " + data["MessNum"] + " OK";
	else  result = "200 " + data["MessNum"] + " OK"; 

	if (data["CMD"] == "CRCX" || data["CMD"] == "MDCX")
	{
		result += "\nZ: " + data["EventType"] + "/" + data["EventID"] + "@[" + data["Addr"] + "]";
		result += "\nI: " + to_string(rand() % 10000);

		result += "\n\n" + serverSDP;
	}
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MGCP::ResponseBAD()
{
	string result = "400 " + to_string(stoi(data["MessNum"]) + 1) + " BAD";
	result += "\nZ: innerError " + innerError; 
	BOOST_LOG_SEV(LOG::vecLogs, fatal) << result << "\nMassage was:" << mgcp;
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::ReplyNOTMGCP()
{
	string result = "400 999 BAD";
	result += "\nZ: outerError " + outerError;
	BOOST_LOG_SEV(LOG::vecLogs, warning) << result << "\nMassage was:" << mgcp;
	NET::GS(NET::mgcp)->s.send_to(boost::asio::buffer(result), sender);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MGCP::PrintAll()
{
	string result = "";
	for (auto& e : data) result += "\n_" + e.first + "_=_" + e.second + "_";
	result += "\n" + sdp;
	return result;
}