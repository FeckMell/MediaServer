#include "stdafx.h"
#include "Connection.h"
#include "MGCPserver.h"
#include "Utils.h"
#include "DestFusion.h"
#include "SrcCommon.h"

using boost::asio::ip::udp;
namespace asio = boost::asio;

static ThreadedSet<unsigned short> thsetPortsInUse;

CMGCPConnection::CMGCPConnection(const CMGCPServer& server, const MGCP::TMGCP& mgcp)
: m_server(server)
{	
	static unsigned __id = 1;
	if (!mgcp.cllSDPs.empty())
		m_sdpOUT = *mgcp.cllSDPs.front();

	static unsigned long long __idSess = 100000;
	m_sdpOUT.name = "MGCPserver 1.0";
	m_sdpOUT.org.username = "-";
	m_sdpOUT.org.sess_id = __idSess++; //tp.time_since_epoch().count();
	m_sdpOUT.org.sess_ver = __idSess++;// m_sdpOUT.org.sess_id;
	m_sdpOUT.org.nettype = "IN";
	m_sdpOUT.org.addrtype = "IP4";
	m_sdpOUT.org.addr = m_server.EndP_Local().address().to_string();
/*
	m_sdpOUT.connection.nettype = "IN";
	m_sdpOUT.connection.addrtype = "IP4";
	m_sdpOUT.connection.address = m_sdpOUT.org.addr;
*/

/*
	if (!m_sdpOUT.cllMedia.empty())
	{
		auto& media = m_sdpOUT.cllMedia.front();
		media.ptime
	}*/

	assert(mgcp.CMD == MGCP::TMGCP::CRCX);
	m_cllSDPs = mgcp.cllSDPs;
	m_id = __id++;

	m_srcPort = thsetPortsInUse.regnew(16000, 2);
}

//-----------------------------------------------------------------------
CMGCPConnection::~CMGCPConnection()
{
	thsetPortsInUse.unreg(m_srcPort);
}

//-----------------------------------------------------------------------
void CMGCPConnection::sendMedia(const string& strFile)
{
	if (!m_cllSDPs.empty() && !m_cllSDPs.front()->cllMedia.empty())
	{
		auto &media = m_sdpOUT.cllMedia[0];

		if (shpDest_)
			shpDest_->terminate();

		//TODO: need waiting for thread before deletion
		shpDest_.reset(new CDestFusion);
		auto shpSrc = std::make_shared<CSrcCommon>(strFile);
		shpDest_->addSrcRef(shpSrc);

		shpDest_->openRTP({m_sdpOUT.connection.address, media.port, SrcPort(), media.ptime});
		if (!shpDest_->LastError())
			shpDest_->run();
	}
}


void CMGCPConnection::stopMedia()
{
	if (shpDest_)
		shpDest_->terminate();
}

void CMGCPConnection::newSubConnection(unsigned idConn, MGCP::TMGCP &mgcp) 
{
	m_cllSubConns.push_back({ idConn, mgcp.parMode });
}

void CMGCPConnection::Modify(const MGCP::TMGCP & mgcp)
{
	auto idConn = mgcp.idConn;
	if (m_id == idConn)
		;
	else
	{
		for (auto& entry : m_cllSubConns)
		{
			if (entry.id == idConn)
			{
				if (mgcp.parMode != MGCP::TMGCP::ConnMode::non)
					entry.mode = mgcp.parMode;
			}
		}
	}
}
