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
extern string DateStr;
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
	
	TArgs	m_args;
private:
/*�������*/
	void loggit(string a);

/*��������� ��������� ������*/
	void proceedReceiveBuffer();

	udp::socket socket_;
	udp::endpoint sender_endpoint_;
	enum { max_length = 2048 };

	std::mutex  mutex_;
	asio::io_service& io_service__;
	
	std::queue<MGCP> Que;
};

