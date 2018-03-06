#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "ConfControl.h"
#include "Functions.h"
#include "Parser.h"
#include "Conf.h"
/************************************************************************
	CMGCPServer
************************************************************************/
class ConfControl;
extern FILE *FileLogServer;
extern Logger CLogger;
class CMGCPServer
{
public:
	ConfControl* Conference;
	typedef std::lock_guard<std::mutex> lock;
	struct TArgs
	{
		asio::io_service&		io_service; 
		const udp::endpoint&	endpnt;
		string					strMmediaPath;
	};
	
	CMGCPServer(const TArgs&);
	const udp::endpoint& EndP_Local() const { return m_args.endpnt; }
	void Run();
	void reply(const string&, const udp::endpoint&);
	void loggit(string a);
	TArgs	m_args;
private:
/*Отладка*/
	//void loggit(string a);

	//void do_receive();
	//void do_send(std::size_t length);
	//void respond(const string);
	//void reply(const string&, const udp::endpoint&);

/*Первичная обработка команд*/
	void proceedReceiveBuffer(const char*, const udp::endpoint&);

	//TArgs	m_args;
	udp::socket socket_;
	udp::endpoint sender_endpoint_;
	enum { max_length = 2048 };
	char data_[max_length + 1];

	std::mutex  mutex_;
	asio::io_service& io_service__;
	
	
	//boost::scoped_ptr<std::thread> ThreadAddPoint;

};

