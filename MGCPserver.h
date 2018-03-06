#pragma once
#include "PackMGCP.h"


//struct TMGCP;

/************************************************************************
	CMGCPServer
************************************************************************/
class CMGCPConnection;
typedef MGCP::TEndPoint KEY_MGCPConnection;
typedef shared_ptr<CMGCPConnection> SHP_CMGCPConnection;
typedef std::map<KEY_MGCPConnection, SHP_CMGCPConnection> MAP_CMGCPConnections;

class CMGCPServer
{
public:
	struct TArgs
	{
		asio::io_service&		io_service; 
		const udp::endpoint&	endpnt;
		string					strMmediaPath;
	};
	CMGCPServer(const TArgs&);

	const udp::endpoint& EndP_Local() const { return m_args.endpnt; }

	void Run();

private:
	void do_receive();
	void do_send(std::size_t length);
	void respond(const string);

	void proceedReceiveBuffer(const char*, const udp::endpoint&);
	void proceedDLCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO);
	void proceedRQNT(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO);
	void proceedCRCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO);
	void proceedMDCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO);

	void reply(const string&, const udp::endpoint&);
	SHP_CMGCPConnection getConnection(const MGCP::TMGCP&, const udp::endpoint&);
	SHP_CMGCPConnection findConnection(const MGCP::TMGCP& mgcp) const;
	TArgs	m_args;
	udp::socket socket_;
	udp::endpoint sender_endpoint_;
	enum { max_length = 2048 };
	char data_[max_length + 1];

	//udp::endpoint	m_epLocal;
	MAP_CMGCPConnections m_cllConnections;
};
