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

	MGCPConference = new MGCPControl();
	MGCPConference->server = this;
	MGCPConference->my_IP = args.MGCPendpnt.address().to_string();
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
		return;
	}
	default: { loggit("reply unknown state"); return; }
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


