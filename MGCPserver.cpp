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
	CLogger.AddToLog(3, "\n" + time + "       " + a + "\n//-------------------------------------------------------------------");
}
//--------------------------------------------------------------------------------------------
CMGCPServer::CMGCPServer(const TArgs& args)
: m_args(args), io_service__(args.io_service), socket_(args.io_service, /*udp::endpoint(udp::v4(), args.endpnt.port())*/args.endpnt)
{
	ConfControl* ff = new ConfControl;
	Conference = ff;
	Conference->server = this;
	Conference->my_IP = args.endpnt.address().to_string();
}
//--------------------------------------------------------------------------------------------
void CMGCPServer::Run()
{
	loggit("Server.Run()");
	while (true)
	{
		udp::endpoint sender_endpoint;
		loggit("Waiting for Callagent request...");
		cout << "\nWaiting for Callagent request...";
		char data_[max_length + 1];

		size_t bytes_recvd = socket_.receive_from(
			asio::buffer(data_, max_length), sender_endpoint);
		data_[bytes_recvd] = 0;
		boost::thread my_thread(&CMGCPServer::proceedReceiveBuffer, this, data_, sender_endpoint);
		my_thread.detach();
	}
}
//--------------------------------------------------------------------------------------------
void CMGCPServer::reply(const string& str, const udp::endpoint& udpTO)
{
	socket_.send_to(asio::buffer(str), udpTO);
	loggit("void CMGCPServer::reply:\n" + str);
}
//--------------------------------------------------------------------------------------------
void CMGCPServer::proceedReceiveBuffer(const char* pCh, const udp::endpoint& udpTO)
{
	loggit(str(boost::format("\n%1% sent:\n%2%") % udpTO % pCh));
	
	string message(pCh);
	auto tp1 = steady_clock::now();
	MGCP mgcp(message);
	auto tp2 = steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(tp2 - tp1);

	loggit("Parsing duration: " + to_string(duration.count()) + "microseconds");
	cout << "\nMessage: " + mgcp.CMD + " " + mgcp.EventEx << boost::format(" Parsing duration: %1% microseconds\n") % duration.count();
	
	if (mgcp.error == -1) { reply("Not MGCP", udpTO); return; }
	if (mgcp.CMD == "CRCX"){ Conference->proceedCRCX(mgcp, udpTO); }
	else if (mgcp.CMD == "MDCX"){ Conference->proceedMDCX(mgcp, udpTO); }
	else if (mgcp.CMD == "RQNT"){ Conference->proceedRQNT(mgcp, udpTO); }
	else if (mgcp.CMD == "DLCX"){ Conference->proceedDLCX(mgcp, udpTO); }
	else {reply("error 504 Unknown or unsupported command", udpTO);}
}
//--------------------------------------------------------------------------------------------




