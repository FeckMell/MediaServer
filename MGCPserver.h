#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "MGCPControl.h"
#include "SIPControl.h"
#include "Functions.h"
#include "Parser.h"
#include "ConfRoom.h"
typedef std::shared_ptr<char*> SHP_char;
/************************************************************************
	CMGCPServer
************************************************************************/
class MGCPControl;
extern string DateStr;
extern Logger CLogger;
class CMGCPServer
{
public:
	//typedef std::lock_guard<std::mutex> lock;
	enum Event_type { mgcp, sip };
	enum { max_length = 2048 };

	struct TArgs
	{
		asio::io_service&		io_service; 
		const udp::endpoint&	endpnt;
		const udp::endpoint&	SIPendpnt;
		string					strMmediaPath;
	};
	struct SIPdata
	{
		SHP_Socket socket_;
		SIP data;
		std::mutex  mutex_;
		std::queue<SIP> Que;
	};
	struct MGCPdata
	{
		SHP_Socket socket_;
		MGCP data;
		std::mutex  mutex_;
		std::queue<MGCP> Que;
	};
	
	CMGCPServer(const TArgs&);
	void RunBuffer();
	void Run();
	void reply(const string&, const udp::endpoint&, Event_type);
	
	TArgs	m_args;
private:
/*Отладка*/
	void loggit(string a);
	
/*Первичная обработка команд*/
	void proceedReceiveBuffer();
	void receive_h(boost::system::error_code, size_t, Event_type);
	
	SIPdata SIPst;
	MGCPdata MGCPst;

	MGCPControl* Conference;
	asio::io_service& io_service__;
};

