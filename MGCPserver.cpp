#include "stdafx.h"
#include "MGCPserver.h"
#include "Connection.h"
#include "MGCPparser.h"
//#include "conf.cpp"
#include "Utils.h"


/*
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
*/
#include <boost/algorithm/string.hpp>
#include <boost/scope_exit.hpp>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <thread>
//#define BOOST_REGEX_MATCH_EXTRA
//#include <boost/regex.hpp>
/*
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/find_iterator.hpp>

*/
namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;

template <typename Iterator>
bool parse_filename(Iterator first, Iterator last, string& strOut)
{
	using qi::char_;

	qi::rule<Iterator, string()> _file;
	_file = *(char_ - char_("()"));

	bool b = qi::parse(first, last,
		*(*(char_ - ',') >> ',') >> "file://"
		>> _file[phoenix::ref(strOut) = qi::_1]
		);

	//b = first == last;
	return b;
}

static ThreadedSet<unsigned> thsetAnnIdInUse;
static ThreadedSet<unsigned> thsetCnfIdInUse;

/************************************************************************
	CMGCPServer
************************************************************************/
CMGCPServer::CMGCPServer(const TArgs& args)
: m_args(args), socket_(args.io_service, udp::endpoint(udp::v4(), args.endpnt.port()))
{

}

//-----------------------------------------------------------------------
void CMGCPServer::do_receive()
{
	socket_.async_receive_from(
		asio::buffer(data_, max_length), sender_endpoint_,
		[this](boost::system::error_code ec, std::size_t bytes_recvd)
	{
		/*
		if (!ec && bytes_recvd > 0)
		do_send(bytes_recvd);
		else
		do_receive();
		*/
		data_[bytes_recvd] = 0;
		string strTmp(data_);
		boost::replace_all(strTmp, "\n", "!\n");
		boost::replace_all(strTmp, "\r!", "!");

		printf("--------- %s sent:\n%s\n--------- len %lu ---------\n\n",
			sender_endpoint_.address().to_string().c_str(), strTmp.c_str(), bytes_recvd);
		proceedReceiveBuffer(data_, sender_endpoint_);

		//do_receive();
	});
}

//-----------------------------------------------------------------------
void CMGCPServer::do_send(std::size_t length)
{
	socket_.async_send_to(
		asio::buffer(data_, length), sender_endpoint_,
		[this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/)
	{
		do_receive();
	});
}

//-----------------------------------------------------------------------
SHP_CMGCPConnection CMGCPServer::findConnection(const MGCP::TMGCP& mgcp) const
{
	auto itr = m_cllConnections.find(mgcp.EndPoint);
	if (itr == m_cllConnections.cend())
		return SHP_CMGCPConnection();

	return itr->second;
}

//-----------------------------------------------------------------------
SHP_CMGCPConnection CMGCPServer::getConnection(const MGCP::TMGCP& mgcp, const udp::endpoint& udpTO)
{
	auto conn = findConnection(mgcp);
	if (!conn)
		//error 500 The transaction could not be executed, because the endpoint is unknown.
		reply(str(boost::format("500 %1% unknown endpoint") % mgcp.IdTransact), udpTO);

	return conn;
}


//-----------------------------------------------------------------------
void CMGCPServer::proceedCRCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	using namespace MGCP;
	assert(mgcp.CMD == TMGCP::CRCX);

	string strResponse;
	BOOST_SCOPE_EXIT(&strResponse, &mgcp, &udpTO, this_){
		if (strResponse.empty())
			strResponse = mgcp.ResponseOK();
		this_->reply(strResponse, udpTO);
	}BOOST_SCOPE_EXIT_END;


	auto conn = findConnection(mgcp);
	if (conn)
	{
		const unsigned idConn = thsetCnfIdInUse.regnew(1, 1);
		conn->newSubConnection(idConn, mgcp);
		mgcp.addResponseParam({ TMGCP_Param::ConnectionId, str(boost::format("%1%") % idConn) });
		return;
	}

	if (mgcp.EndPoint.m_point == "cnf/$")
	{
		TEndPoint mgcpEndPnt = {
			str(boost::format("cnf/%1%") % thsetCnfIdInUse.regnew(1, 1)),
			mgcp.EndPoint.m_addr };
		auto shpConnect = std::make_shared<CMGCPConnection>(*this, mgcp);
		m_cllConnections[mgcpEndPnt] = shpConnect;

		mgcp.addResponseParam({ TMGCP_Param::SpecificEndPointId, mgcpEndPnt.toString() });
		mgcp.addResponseParam({ TMGCP_Param::ConnectionId,
			str(boost::format("%1%") % shpConnect->Id()) });

		return;
	}


	if (mgcp.EndPoint.m_point == "ann/$")
	{
		TEndPoint mgcpEndPnt = {
			str(boost::format("ann/%1%") % thsetAnnIdInUse.regnew(1, 1)),
			mgcp.EndPoint.m_addr };
		auto shpConnect = std::make_shared<CMGCPConnection>(*this, mgcp);
		m_cllConnections[mgcpEndPnt] = shpConnect;

		mgcp.addResponseParam({ TMGCP_Param::SpecificEndPointId, mgcpEndPnt.toString() });
		mgcp.addResponseParam({ TMGCP_Param::ConnectionId,
			str(boost::format("%1%") % shpConnect->Id()) });
		//			auto &media = shpConnect->sdpOUT().cllMedia[0];
		/*
					auto strResponse = mgcp.ResponseOK() + shpConnect->sdpOUT().toString();
					for (auto& entry : shpConnect->sdpOUT().cllParams)
					strResponse += str(boost::format("\n%1%=%2%") % entry->m_type % entry->m_value);
					*/
		auto f = boost::format(
			string("\n\nv=0\n"
			"o=- 0 0 IN IP4 %1%\n"
			"s=MGCP_server\n"
			"c=IN IP4 %1%\n"
			"t=0 0\n"
			//"a=tool:libavformat 57.3.100\n"
			"m=audio %2% RTP/AVP 8\n"
			"a=rtpmap:8 PCMA/8000\n"
			"a=sendonly"
			//"a=recvonly"
			//"\nb=AS:64"
			));

		strResponse = mgcp.ResponseOK() + str(f %
			EndP_Local().address().to_string() % shpConnect->SrcPort());

		return;
	}
}

//-----------------------------------------------------------------------
void CMGCPServer::proceedDLCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	assert(mgcp.CMD == MGCP::TMGCP::DLCX);
	if (SHP_CMGCPConnection conn = getConnection(mgcp, udpTO))
	{
		conn->stopMedia();
		reply(mgcp.ResponseOK(250), udpTO);

		const auto& epMGCP = mgcp.EndPoint;
		unsigned id = 0;
		if(qi::parse(epMGCP.m_point.cbegin(), epMGCP.m_point.cend(),
				"ann/" >> qi::uint_[phoenix::ref(id) = qi::_1]))
		{
			thsetAnnIdInUse.unreg(id);
		}
		if (!id && qi::parse(epMGCP.m_point.cbegin(), epMGCP.m_point.cend(),
			"cnf/" >> qi::uint_[phoenix::ref(id) = qi::_1]))
		{
			thsetCnfIdInUse.unreg(id);
		}

		m_cllConnections.erase(epMGCP);
	}
}

//-----------------------------------------------------------------------
void CMGCPServer::proceedMDCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	assert(mgcp.CMD == MGCP::TMGCP::MDCX);
	if (SHP_CMGCPConnection conn = getConnection(mgcp, udpTO))
	{
		conn->Modify(mgcp);
		reply(mgcp.ResponseOK(), udpTO);
	}
}

void threadSendMedia(SHP_CMGCPConnection conn, string strFile)
{	
	//printf("\n test333\n");
	conn->sendMedia(strFile);
}

//-----------------------------------------------------------------------
void CMGCPServer::proceedRQNT(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	assert(mgcp.CMD == MGCP::TMGCP::RQNT);
	if (SHP_CMGCPConnection conn = getConnection(mgcp, udpTO))
	{
		reply(mgcp.ResponseOK(), udpTO);

		if (auto param = mgcp.getSignalParam())
		{
			string strFile;
			if(parse_filename(param->m_value.cbegin(), param->m_value.cend(), strFile))
			{
				//TODO: multithreading & concurrent issue
				std::thread th(threadSendMedia, conn, m_args.strMmediaPath + strFile);
				th.detach();
/*
				non-stable in UNIX
				std::thread th([&](SHP_CMGCPConnection c)
				{
				c->sendMedia(m_args.strMmediaPath + strFile);
				},
				conn);*/
/*
				conn->setMediaFile(m_args.strMmediaPath + strFile);
				std::thread th([](SHP_CMGCPConnection c){c->sendMedia(); }, conn);
				th.detach();*/

			}
		}
	}
}

//-----------------------------------------------------------------------
void CMGCPServer::proceedReceiveBuffer(const char* pCh, const udp::endpoint& udpTO)
{
	using namespace MGCP;
	const auto start_tp = chrono::high_resolution_clock::now();
 
	cout << boost::format("=================== %1% sent:\n%2%\n===================\n") % udpTO % pCh;

	TMGCP mgcp;
	if (!parseMGCP(pCh, mgcp))
	{
		reply("Not MGCP", udpTO);
		//socket_.send_to(asio::buffer("Not MGCP"), udpTO);
		return;
	}

/*
	const char *pLine = strpbrk(pCh, "\n");
	if (!pLine)
		pLine = pCh;
	cout << boost::format("%1% sent: %2%...\n") % udpTO %  string(pCh, pLine);
*/


	const auto end_tp = chrono::high_resolution_clock::now();
	const auto dur = chrono::duration_cast<chrono::microseconds>(end_tp - start_tp);
	cout << boost::format("Parsing duration: %1% microseconds\n") % dur.count();

	switch (mgcp.CMD)
	{
	case TMGCP::CRCX:
		proceedCRCX(mgcp, udpTO);
		break;
	case TMGCP::RQNT:
		proceedRQNT(mgcp, udpTO);
		break;
	case TMGCP::DLCX:
		proceedDLCX(mgcp, udpTO);
		break;
	case TMGCP::MDCX:
		proceedMDCX(mgcp, udpTO);
		break;
	default:
		//error 504 Unknown or unsupported command.
		reply(str(boost::format("504 %1% unsupported command") % mgcp.IdTransact), udpTO);
		break;
	}
}

//-----------------------------------------------------------------------
void CMGCPServer::reply(const string& str, const udp::endpoint& udpTO)
{
	socket_.send_to(asio::buffer(str), udpTO);
	cout << boost::format("----------Replied to %1%:\n%2%\n------------------\n")
		% udpTO % str;
}

//-----------------------------------------------------------------------
void CMGCPServer::respond(const string str)
{
	socket_.async_send_to(
		asio::buffer(str), sender_endpoint_,
		[this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/)
	{
		do_receive();
	});
}

//-----------------------------------------------------------------------
void CMGCPServer::Run()
{
	for (;;)
	{
		udp::endpoint sender_endpoint;
		cout << "Waiting for Callagent request...\n";

		size_t bytes_recvd = socket_.receive_from(
			asio::buffer(data_, max_length), sender_endpoint);

		data_[bytes_recvd] = 0;
#ifdef _DEBUG
		if (strcmp(data_, "exit") == 0)
		{
			reply("Bye!", sender_endpoint);
			break;
		}
#endif // _DEBUG

		const char* pLogData = data_;
		/*
		string strTmp(data_);
		boost::replace_all(strTmp, "\n", "!\n");
		boost::replace_all(strTmp, "\r!", "!");
		pLogData = strTmp.c_str();
		*/



//		printf("--------- %s sent:\n%s\n--------- len %lu ---------\n\n",
//			sender_endpoint.address().to_string().c_str(), pLogData, bytes_recvd);


		proceedReceiveBuffer(data_, sender_endpoint);
	}
}
