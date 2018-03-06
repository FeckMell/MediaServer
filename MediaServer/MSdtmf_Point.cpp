#include "stdafx.h"
#include "MSdtmf_Point.h"
using namespace dtmf;


Point::Point(SHP_IPL ipl_)
{
	
	serverPort = ipl_->data["ServerPort"];
	clientPort = ipl_->data["ClientPort"];
	clientIP = ipl_->data["ClientIP"];
	callID = ipl_->data["CallID"];
	eventID = ipl_->data["EventID"];
	
	socket = SSTORAGE::GetSocket(serverPort);
	endPoint = EP(boost::asio::ip::address::from_string(clientIP), stoi(clientPort));
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Point::Analyze(uint8_t ch[2])
{
	//
	if (ch[0] == (uint8_t)229)
	{
		
		int button = (int)ch[1];
		if (button < 10 && button >= 0)
		{
			buttons.push_back(button);
			return false;
		}
		else if (button==11) 
		{
			SendModul();
			return true;
		}
		else if (button ==10)
		{
			buttons.clear();
			return false;
		}
		else return false;
	}
	else
	{
		//
		return false;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::SendModul()
{
	string s_but = "";
	for (auto&e : buttons) s_but += to_string(e);

	string result = "";
	result += "From=dtmf\n";
	result += "To=sip\n";
	result += "EventID=" + eventID + "\n";
	result += "CallID=" + callID + "\n";
	result += "Data=" + s_but + "\n";
	NET::vecSigsIN[NET::INNER::sip](result);
}
