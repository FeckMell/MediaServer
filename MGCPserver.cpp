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
	CLogger.AddToLog(3, "\n" + time + "       " + a + "\n//-------------------------------------------------------------------");
}
//-*/----------------------------------------------------------
CMGCPServer::CMGCPServer(const TArgs& args)
: m_args(args), io_service__(args.io_service)//, socket_(args.io_service, /*udp::endpoint(udp::v4(), args.endpnt.port())*/args.endpnt)
{
	socket_.reset(new ip::udp::socket(io_service__));
	socket_->open(udp::v4());
	socket_->set_option(ip::udp::socket::reuse_address(true));
	socket_->bind(args.endpnt);
	RequestControl* ff = new RequestControl;
	Conference = ff;
	Conference->server = this;
	Conference->my_IP = args.endpnt.address().to_string();
}
//-*/----------------------------------------------------------
void CMGCPServer::RunBuffer()
{
	loggit("Buffer start!");
	boost::thread th(&CMGCPServer::proceedReceiveBuffer, this);
	th.detach();
}
//-*/----------------------------------------------------------
void CMGCPServer::Run()
{
	loggit("Server.Run()");
	
	while (true)
	{
		loggit("Waiting for Callagent request...");
		cout << "\nWaiting for Callagent request...";
		MGCP mgcp;
		size_t bytes_recvd = socket_->receive_from(asio::buffer(mgcp.mes, max_length), mgcp.sender);

		mgcp.mes[bytes_recvd] = 0;
		loggit(str(boost::format("Client %1% sent:\n%2%") % mgcp.sender % mgcp.mes));

		mutex_.lock();
		Que.push(mgcp);
		mutex_.unlock();
	}
}
//-*/----------------------------------------------------------
void CMGCPServer::reply(const string& strr, const udp::endpoint& udpTO)
{
    socket_->send_to(asio::buffer(strr), udpTO);
	loggit("CMGCPServer reply to " + str(boost::format("%1%") % udpTO) + ":\n" + strr);
}
//-*/----------------------------------------------------------
void CMGCPServer::proceedReceiveBuffer()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (Que.size() > 0)
		{
			auto mgcp = Que.front();
			mutex_.lock();
			Que.pop();
			mutex_.unlock();
			auto tp1 = steady_clock::now();
			mgcp.Parse(true);
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(steady_clock::now() - tp1);
			loggit("\nParsing duration: " + to_string(duration.count()) + "milliseconds");
			cout << "\nMessage: " + mgcp.CMD + " " + mgcp.EventEx << boost::format(" Parsing duration: %1% milliseconds\n") % duration.count();

			if (mgcp.error == -1) { reply("Not MGCP", mgcp.sender); }
			else if (mgcp.CMD == "CRCX"){ Conference->proceedCRCX(mgcp); }
			else if (mgcp.CMD == "MDCX"){ Conference->proceedMDCX(mgcp); }
			else if (mgcp.CMD == "RQNT"){ Conference->proceedRQNT(mgcp); }
			else if (mgcp.CMD == "DLCX"){ Conference->proceedDLCX(mgcp); }
			else { reply("error 504 Unknown or unsupported command", mgcp.sender); }
			loggit("this request done, going to next");
		}
	}
}
//-*/----------------------------------------------------------



