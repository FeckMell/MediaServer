#pragma once
#include "stdafx.h"
#include "Functions.h"
#include "MGCPserver.h"
extern boost::gregorian::date Date;
extern string DateStr;
extern string PathEXE;

/************************************************************************
*************************Handlers****************************************
************************************************************************/

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
	time += to_string(t->tm_year + 1900) + "." + to_string(t->tm_mon + 1) + "." + to_string(t->tm_mday) + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);

	//fprintf(FileLogServer, ("\n" + time + "       " + a + "\n//-------------------------------------------------------------------").c_str());
	//fflush(FileLogServer);
	CLogger.AddToLog(3, "\n" + time + "       " + a);
}
//--------------------------------------------------------------------------------------------
CMGCPServer::CMGCPServer(const TArgs& args)
: m_args(args), io_service__(args.io_service), socket_(args.io_service, /*udp::endpoint(udp::v4(), args.endpnt.port())*/args.endpnt)
{
	//Conference->my_IP = args.endpnt.address().to_string();
	ConfControl* ff = new ConfControl;
	//ConfControl ff;
	Conference = ff;
	Conference->server = this;
	Conference->my_IP = args.endpnt.address().to_string();
	//fopen_s(&FileLogServer, "LOGS_Server.txt", "w");
	loggit("Server Construct");
}
//--------------------------------------------------------------------------------------------
void CMGCPServer::Run()
{
	loggit("Server.Run()");
	for (;;)
	{
		udp::endpoint sender_endpoint;
		loggit("Waiting for Callagent request...");
		cout << "\nWaiting for Callagent request...";

		size_t bytes_recvd = socket_.receive_from(
			asio::buffer(data_, max_length), sender_endpoint);//

		data_[bytes_recvd] = 0;//

#ifdef _DEBUG
		if (strcmp(data_, "exit") == 0)
		{
			reply("Bye!", sender_endpoint);
			break;//
		}
#endif // _DEBUG

		const char* pLogData = data_;

		proceedReceiveBuffer(data_, sender_endpoint);//
	}
}
//--------------------------------------------------------------------------------------------
/*void CMGCPServer::do_receive()
{
	loggit("do_receive");
	out << "\n???";
	socket_.async_receive_from(
		asio::buffer(data_, max_length), sender_endpoint_,
		[this](boost::system::error_code ec, std::size_t bytes_recvd)
	{
		data_[bytes_recvd] = 0;
		string strTmp(data_);
		boost::replace_all(strTmp, "\n", "!\n");
		boost::replace_all(strTmp, "\r!", "!");
		loggit("void CMGCPServer::do_receive()\n---------" + sender_endpoint_.address().to_string() + " sent:\n" + strTmp);
		printf("--------- %s sent:\n%s\n--------- len %lu ---------\n\n",
			sender_endpoint_.address().to_string().c_str(), strTmp.c_str(), bytes_recvd);
		proceedReceiveBuffer(data_, sender_endpoint_);
	});
}  */
//--------------------------------------------------------------------------------------------
/*void CMGCPServer::do_send(std::size_t length)
{
	out << "\n!!!";
	socket_.async_send_to(
		asio::buffer(data_, length), sender_endpoint_,
		[this](boost::system::error_code , std::size_t )
	{
		loggit("void CMGCPServer::do_send");
		do_receive();
	});
}*/
//--------------------------------------------------------------------------------------------
/*void CMGCPServer::respond(const string str)
{
	out << "\n@@@@";
	socket_.async_send_to(
		asio::buffer(str), sender_endpoint_,
		[this](boost::system::error_code , std::size_t )
	{
		loggit("void CMGCPServer::respond");
		do_receive();
	});
}*/
//--------------------------------------------------------------------------------------------
void CMGCPServer::reply(const string& str, const udp::endpoint& udpTO)
{
	socket_.send_to(asio::buffer(str), udpTO);
	//auto f = boost::format("----------REPLIED TO %1%:\n%2%\n------------------") % udpTO % str;
	
	//out << "\n --------------------Server REPLIED------------------------\n";
	loggit("void CMGCPServer::reply:\n" + str);
	//out << boost::format("----------REPLIED TO %1%:\n%2%\n------------------\n")% udpTO % str;
}
//--------------------------------------------------------------------------------------------
void CMGCPServer::proceedReceiveBuffer(const char* pCh, const udp::endpoint& udpTO)
{
	loggit("void CMGCPServer::proceedReceiveBuffer()");
	//cout << boost::format("=================== %1% sent:\n%2%\n===================\n") % udpTO % pCh; // верхн€€ строчка короче
	loggit(str(boost::format("\n=================== %1% sent:\n%2%") % udpTO % pCh));
	string message(pCh);
	auto tp1 = steady_clock::now();
	MGCP mgcp(message);
	auto tp2 = steady_clock::now();
	cout << "\nMessage: " + mgcp.CMD + " " + mgcp.EventEx + " ";
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(tp2 - tp1);
	loggit("Parsing duration: " + to_string(duration.count()) + "microseconds");
	cout << boost::format("Parsing duration: %1% microseconds\n") % duration.count();
	if (mgcp.error == -1) { reply("Not MGCP", udpTO); return; }
	if (mgcp.CMD == "CRCX"){ Conference->proceedCRCX(mgcp, udpTO); }
	else if (mgcp.CMD == "MDCX"){ Conference->proceedMDCX(mgcp, udpTO); }
	else if (mgcp.CMD == "RQNT"){ Conference->proceedRQNT(mgcp, udpTO); }
	else if (mgcp.CMD == "DLCX"){ Conference->proceedDLCX(mgcp, udpTO); }
	else {reply("error 504 Unknown or unsupported command", udpTO);}
}
//--------------------------------------------------------------------------------------------




