#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "RequestControl.h"
#include "Functions.h"
#include "Parser.h"
#include "ConfRoom.h"
/************************************************************************
	CMGCPServer
************************************************************************/
class RequestControl;
extern string DateStr;
extern Logger CLogger;
class CMGCPServer
{
public:
	RequestControl* Conference;
	typedef std::lock_guard<std::mutex> lock;
	struct TArgs
	{
		asio::io_service&		io_service; 
		const udp::endpoint&	endpnt;
		string					strMmediaPath;
	};
	
	CMGCPServer(const TArgs&);
	const udp::endpoint& EndP_Local() const { return m_args.endpnt; }
	void RunBuffer();
	void Run();
	void reply(const string&, const udp::endpoint&);
	
	TArgs	m_args;
private:
/*Отладка*/
	void loggit(string a);

/*Первичная обработка команд*/
	void proceedReceiveBuffer();
	SHP_Socket socket_;
	udp::endpoint sender_endpoint_;
	enum { max_length = 2048 };

	std::mutex  mutex_;
	asio::io_service& io_service__;
	
	std::queue<MGCP> Que;
};

