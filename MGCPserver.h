#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "MGCPControl.h"
#include "SIPControl.h"
#include "Functions.h"
#include "Parser.h"
#include "ConfRoom.h"
/************************************************************************
	CMGCPServer
************************************************************************/
class MGCPControl;
class SIPControl;
extern string DateStr;
extern Logger* CLogger;
class CMGCPServer
{
public:
	

	MGCPControl* MGCPConference;
	SIPControl* SIPConference;
	

	enum Event_type { mgcp, sip };
	enum { max_length = 2048 };

	struct TArgs
	{
		asio::io_service&		io_service; 
		const udp::endpoint&	MGCPendpnt;
		const udp::endpoint&	SIPendpnt;
		string					strMmediaPath;
	};
	struct SIPdata
	{
		SHP_Socket socket;
		SIP data;
		std::mutex  mutex_;
		std::queue<SIP> Que;
	};
	struct MGCPdata
	{
		SHP_Socket socket;
		MGCP data;
		std::mutex  mutex_;
		std::queue<MGCP> Que;
	};
	
	CMGCPServer(const TArgs&);
	const udp::endpoint& EndP_Local() const { return m_args.MGCPendpnt; }
	void RunBuffer();
	void Run();
	void reply(const string&, const udp::endpoint&, Event_type);

	int GetFreePort();
	void SetFreePort(int port);

	TArgs	m_args;
private:
/*Отладка*/
	void loggit(string a);

/*Первичная обработка команд*/
	void receive_h(boost::system::error_code, size_t, Event_type);
	void proceedReceiveBuffer();
	SIPdata SIPst;
	MGCPdata MGCPst;
	udp::endpoint sender_endpoint_;
	

	//std::mutex  mutex_;
	asio::io_service& io_service__;
	
	//std::queue<MGCP> Que;
};

