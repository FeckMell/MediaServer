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
	socket->SetEndPoint(clientIP, clientPort);
}
void Point::Run()
{
	socket->AsyncReceive(boost::bind(&Point::Receive, this, _1, _2));
	th.reset(new thread(&Point::RunIO, this));
}
void Point::RunIO()
{
	socket->io->reset();
	socket->io->run();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Point::~Point()
{
	state = false;
	socket->s.cancel();
	socket->io->reset();
	th->join();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::Receive(boost::system::error_code ec_, size_t size_)
{
	if (state == true)
	{
		bool result = false;
		if (size_ > 12)
		{
			uint8_t bytes[2];

			memcpy(&bytes[0], socket->buffer + 1, 1);
			memcpy(&bytes[1], socket->buffer + 12, 1);

			result = Analyze(bytes);
		}
		if (result == false)
		{
			socket->AsyncReceive(boost::bind(&Point::Receive, this, _1, _2));
		}
		else
		{
			state = false;
			SendModul();
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Point::Analyze(uint8_t ch[2])
{
	if (ch[0] == (uint8_t)229)
	{
		int button = (int)ch[1];
		if (button < 10 && button >= 0)
		{
			buttons.push_back(button);
			return false;
		}
		else if (button ==10)
		{
			buttons.clear();
			return false;
		}
		else if (button == 11)
		{
			return true;
		}
		else return false;
	}
	else return false;
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
	NET::outerIO->post(boost::bind(NET::SignalInIO, NET::INNER::sip, result));
}
