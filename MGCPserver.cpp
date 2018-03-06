#pragma once
#include "stdafx.h"
#include "Functions.h"
#include "MGCPserver.h"

extern boost::gregorian::date Date;
extern string DateStr;
extern string PathEXE;

/************************************************************************
***************************CMGCPServer***********************************
************************************************************************/
void CMGCPServer::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	string time = "";
	steady_clock::time_point t1 = steady_clock::now();
	time += DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	time += " thread=" + boost::to_string(this_thread::get_id());
	CLogger->AddToLog(3, "\n" + time + "       " + a + "\n//-------------------------------------------------------------------");
}
//*///------------------------------------------------------------------------------------------
CMGCPServer::CMGCPServer(const TArgs& args)
: m_args(args), io_service__(args.io_service)//, socket_(args.io_service, /*udp::endpoint(udp::v4(), args.endpnt.port())*/args.endpnt)
{
	MGCPst.socket.reset(new ip::udp::socket(io_service__));
	MGCPst.socket->open(udp::v4());
	MGCPst.socket->set_option(ip::udp::socket::reuse_address(true));
	MGCPst.socket->bind(args.MGCPendpnt);
	
	SIPst.socket.reset(new ip::udp::socket(io_service__));
	SIPst.socket->open(udp::v4());
	SIPst.socket->set_option(ip::udp::socket::reuse_address(true));
	SIPst.socket->bind(args.SIPendpnt);

	MGCPConference = new MGCPControl;
	MGCPConference->server = this;
	MGCPConference->my_IP = args.MGCPendpnt.address().to_string();

	SIPConference = new SIPControl;
	SIPConference->server = this;
	SIPConference->my_IP = args.MGCPendpnt.address().to_string();
}
//*///------------------------------------------------------------------------------------------
void CMGCPServer::RunBuffer()
{
	loggit("Buffer start!");
	boost::thread th(&CMGCPServer::proceedReceiveBuffer, this);
	th.detach();
}
//*///------------------------------------------------------------------------------------------
void CMGCPServer::Run()
{
	loggit("Server.Run()");
	MGCPst.socket->async_receive_from(boost::asio::buffer(MGCPst.data.mes, max_length), MGCPst.data.sender,
		boost::bind(&CMGCPServer::receive_h, this, _1, _2, CMGCPServer::mgcp));
	SIPst.socket->async_receive_from(boost::asio::buffer(SIPst.data.mes, max_length), SIPst.data.sender,
		boost::bind(&CMGCPServer::receive_h, this, _1, _2, CMGCPServer::sip));
}
void CMGCPServer::receive_h(boost::system::error_code ec, size_t szPack, Event_type Event)
{
	switch (Event)
	{
	case CMGCPServer::mgcp:
	{
		if (szPack > 10)
		{
			MGCPst.data.mes[szPack] = 0;
			loggit(str(boost::format("Client %1% sent:\n%2%") % MGCPst.data.sender % MGCPst.data.mes));
			MGCPst.mutex_.lock();
			MGCPst.Que.push(MGCPst.data);
			MGCPst.mutex_.unlock();
		}
		cout << "\nWaiting for next MGCP request...";
		MGCPst.socket->async_receive_from(boost::asio::buffer(MGCPst.data.mes, max_length), MGCPst.data.sender,
			boost::bind(&CMGCPServer::receive_h, this, _1, _2, CMGCPServer::mgcp));
		break;
	}
	case CMGCPServer::sip:
	{
		if (szPack > 10)
		{
			SIPst.data.mes[szPack] = 0;
			loggit(str(boost::format("Client %1% sent:\n%2%") % SIPst.data.sender % SIPst.data.mes));
			SIPst.mutex_.lock();
			SIPst.Que.push(SIPst.data);
			SIPst.mutex_.unlock();
		}
		cout << "\nWaiting for next SIP request...";
		SIPst.socket->async_receive_from(boost::asio::buffer(SIPst.data.mes, max_length), SIPst.data.sender,
			boost::bind(&CMGCPServer::receive_h, this, _1, _2, CMGCPServer::sip));
		break;
	}
	default: { loggit("unknown type receive"); break; }
	}

}
//*///------------------------------------------------------------------------------------------
void CMGCPServer::reply(const string& strr, const udp::endpoint& udpTO, Event_type t)
{
	switch (t)
	{
	case CMGCPServer::mgcp:
	{
		MGCPst.socket->send_to(asio::buffer(strr), udpTO);
		loggit("CMGCPServer MGCP reply to " + str(boost::format("%1%") % udpTO) + ":\n" + strr);
		break;
	}
	case CMGCPServer::sip:
	{
		SIPst.socket->send_to(asio::buffer(strr), udpTO);
		loggit("CMGCPServer SIP reply to " + str(boost::format("%1%") % udpTO) + ":\n" + strr);
		break;
	}
	default: { loggit("reply unknown state"); break; }
	}
}
//*///------------------------------------------------------------------------------------------
void CMGCPServer::proceedReceiveBuffer()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (MGCPst.Que.size() > 0)
		{
			auto mgcp = MGCPst.Que.front();
			MGCPst.mutex_.lock();
			MGCPst.Que.pop();
			MGCPst.mutex_.unlock();
			auto tp1 = steady_clock::now();
			mgcp.Parse(true);
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(steady_clock::now() - tp1);
			loggit("\nParsing duration: " + to_string(duration.count()) + "milliseconds");
			cout << "\nMessage: " + mgcp.CMD + " " + mgcp.EventEx << boost::format(" Parsing duration: %1% milliseconds\n") % duration.count();

			if (mgcp.error == -1) { reply("Not MGCP", mgcp.sender, CMGCPServer::mgcp); }
			else if (mgcp.CMD == "CRCX"){ MGCPConference->proceedCRCX(mgcp); }
			else if (mgcp.CMD == "MDCX"){ MGCPConference->proceedMDCX(mgcp); }
			else if (mgcp.CMD == "RQNT"){ MGCPConference->proceedRQNT(mgcp); }
			else if (mgcp.CMD == "DLCX"){ MGCPConference->proceedDLCX(mgcp); }
			else { reply("error 504 Unknown or unsupported command", mgcp.sender, CMGCPServer::mgcp); }
			loggit("this MGCP request done, going to next");
		}
		if (SIPst.Que.size() > 0)
		{
			auto sip = SIPst.Que.front();
			SIPst.mutex_.lock();
			SIPst.Que.pop();
			SIPst.mutex_.unlock();
			auto tp1 = steady_clock::now();
			sip.Parse();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(steady_clock::now() - tp1);
			loggit("\nParsing duration: " + to_string(duration.count()) + "milliseconds");
			cout << "\nMessage: " + sip.CMD << boost::format(" Parsing duration: %1% milliseconds\n") % duration.count();

			if (sip.error == -1) { reply("Not SIP", sip.sender, CMGCPServer::sip); }
			else if (sip.CMD == SIP::INVITE){ SIPConference->proceedINVITE(sip); }
			else if (sip.CMD == SIP::ACK){ SIPConference->proceedACK(sip); }
			else { reply("error 504 Unknown or unsupported command", sip.sender, CMGCPServer::sip); }

			loggit("this SIP request done, going to next");
		}
	}
}
//*///------------------------------------------------------------------------------------------
int CMGCPServer::GetFreePort()
{
	return MGCPConference->GetFreePort();
}
//*///------------------------------------------------------------------------------------------
void CMGCPServer::SetFreePort(int port)
{
	MGCPConference->SetFreePort(port);
}
//*///------------------------------------------------------------------------------------------


