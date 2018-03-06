#include "stdafx.h"
#include "Point.h"
using namespace dtmf;


Point::Point(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Point::Point(...)";
	serverPort = ipl_->data["ServerPort"];
	clientPort = ipl_->data["ClientPort"];
	clientIP = ipl_->data["ClientIP"];
	callID = ipl_->data["CallID"];
	eventID = ipl_->data["EventID"];
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Point::Point(...) params:\n1) ServerPort=" << serverPort << "\n2) ClientPort=" << clientPort << "\n3) ClientIP=" << clientIP << "\n4) CallID=" << callID << "\n5) EventID=" << eventID;
	socket = SSTORAGE::GetSocket(serverPort);
	endPoint = EP(boost::asio::ip::address::from_string(clientIP), stoi(clientPort));
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Point::Point(...) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Point::Analyze(uint8_t ch[2])
{
	//BOOST_LOG_SEV(LOG::vecLogs, trace) << "Point::Analyze(...) for point=" << eventID;
	if (ch[0] == (uint8_t)229)
	{
		BOOST_LOG_SEV(LOG::vecLogs, trace) << "Point::Analyze(...) ch[0] == (uint8_t)229";
		int button = (int)ch[1];
		if (button < 10)
		{
			BOOST_LOG_SEV(LOG::vecLogs, trace) << "Point::Analyze(...) button < 10 = " << button;
			buttons.push_back(button);
			return false;
		}
		else if (button == 10 || button==11) 
		{
			BOOST_LOG_SEV(LOG::vecLogs, trace) << "Point::Analyze(...) button == 10 = " << button;
			SendModul();
			return true;
		}
		else
		{
			BOOST_LOG_SEV(LOG::vecLogs, trace) << "Point::Analyze(...) button > 10";
			return false;
		}
		
	}
	else
	{
		//BOOST_LOG_SEV(LOG::vecLogs, trace) << "Point::Analyze(...) ch[0] != (uint8_t)229";
		return false;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::SendModul()
{
	string s_but = "";
	for (auto&e : buttons)
		s_but += to_string(e);
	string result = "M7S2I6P5M\n";
	result += "From=dtmf\n";
	result += "To=sip\n";
	result += "EventID=" + eventID + "\n";
	result += "CallID=" + callID + "\n";
	result += "Data=" + s_but + "\n";
	NET::SendModul(NET::INNER::dtmf, NET::INNER::sip_i, result);
}
