#pragma once
#include "PackMGCP.h"
#include "MGCPserver.h"
//#include "Utils.h"
#include "DestFusion.h"
#include "SrcCommon.h"

class CMGCPServer;
class CDestFusion;

class CMGCPConnection
{
public:
	CMGCPConnection(const CMGCPServer&, const MGCP::TMGCP& mgcp);
	~CMGCPConnection();
	unsigned Id() const { return m_id; }
	const MGCP::TSDP& sdpOUT() const { return m_sdpOUT; }
	void sendMedia(const string& strFile, string CallID);
	unsigned short SrcPort() const { return m_srcPort; }
	void stopMedia();
	void newSubConnection(unsigned idConn, MGCP::TMGCP &mgcp);
	void Modify(const MGCP::TMGCP & mgcp);
private:

	const CMGCPServer& m_server;
	MGCP::CLL_SDPs	m_cllSDPs;
	MGCP::TSDP		m_sdpOUT;
	unsigned		m_id;
	unsigned short  m_srcPort;
	std::shared_ptr<CDestFusion> shpDest_;

	struct SubConn
	{
		unsigned id;
		MGCP::TMGCP::ConnMode mode;
	};
	std::vector<SubConn> m_cllSubConns;

};

