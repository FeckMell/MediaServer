#include "stdafx.h"

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
//#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/qi.hpp>
//#include <boost/fusion/sequence.hpp>
//#include <boost/fusion/include/sequence.hpp>

//#include <iostream>
#include "MGCPparser.h"
#include "PackMGCP.h"

namespace MGCP
{
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace fusion = boost::fusion;
namespace phoenix = boost::phoenix;

struct MGCP_Param_symb : qi::symbols<char, TMGCP_Param::ParamType>, boost::noncopyable
{
	static const MGCP_Param_symb& instance(){
		static MGCP_Param_symb single;
		return single;
	}

private:
	MGCP_Param_symb()
	{
		add
			("B", TMGCP_Param::BearerInformation)
			("C", TMGCP_Param::CallId)
			("A", TMGCP_Param::Capabilities)
			("I", TMGCP_Param::ConnectionId)
			("M", TMGCP_Param::ConnectionMode)
			("P", TMGCP_Param::ConnectionParameters)
			("T", TMGCP_Param::DetectEvents)
			("D", TMGCP_Param::DigitMap)
			("ES", TMGCP_Param::EventStates)
			("L", TMGCP_Param::LocalConnectionOptions)
			("MD", TMGCP_Param::MaxMGCPDatagram)
			("N", TMGCP_Param::NotifiedEntity)
			("O", TMGCP_Param::ObservedEvents)
			("PL", TMGCP_Param::PackageList)
			("Q", TMGCP_Param::QuarantineHandling)
			("E", TMGCP_Param::ReasonCode)
			("R", TMGCP_Param::RequestedEvents)
			("F", TMGCP_Param::RequestedInfo)
			("X", TMGCP_Param::RequestIdentifier)
			("K", TMGCP_Param::ResponseAck)
			("RD", TMGCP_Param::RestartDelay)
			("RM", TMGCP_Param::RestartMethod)
			("I2", TMGCP_Param::SecondConnectionId)
			("Z2", TMGCP_Param::SecondEndpointId)
			("S", TMGCP_Param::SignalRequests)
			("Z", TMGCP_Param::SpecificEndPointId)
			;
	}
};

//namespace sdpmedia = TMGCP::TSDP::media;
struct Mediamode_symb : qi::symbols<char, TSDP::media::mmode>, boost::noncopyable
{
	static const Mediamode_symb& instance(){
		static Mediamode_symb single;
		return single;
	}
private:
	Mediamode_symb(){
		add
			("inactive", TSDP::media::inactive)
			("recvonly", TSDP::media::recvonly)
			("sendrecv", TSDP::media::sendrecv)
			("sendonly", TSDP::media::sendonly)
			;
	}
};

struct Conn_Mode_symb : qi::symbols<char, TMGCP::ConnMode>, boost::noncopyable
{
	static const Conn_Mode_symb& instance(){
		static Conn_Mode_symb single;
		return single;
	}

private:
	Conn_Mode_symb()
	{
		add
			("sendonly", TMGCP::sendonly)
			("recvonly", TMGCP::recvonly)
			("sendrecv", TMGCP::sendrecv)
			("confrnce", TMGCP::confrnce)
			("inactive", TMGCP::inactive)
			("loopback", TMGCP::loopback)
			("conttest", TMGCP::conttest)
			("netwloop", TMGCP::netwloop)
			("netwtest", TMGCP::netwtest)
			;
	}
};

struct MGCP_Cmd_symb : qi::symbols<char, TMGCP::MGCPCmd>, boost::noncopyable
{
	static const MGCP_Cmd_symb& instance(){
		static MGCP_Cmd_symb single;
		return single;
	}
private:
	MGCP_Cmd_symb()
	{
		add
			("epcf", TMGCP::EPCF)
			("crcx", TMGCP::CRCX)
			("mdcx", TMGCP::MDCX)
			("dlcx", TMGCP::DLCX)
			("rqnt", TMGCP::RQNT)
			("ntfy", TMGCP::NTFY)
			("auep", TMGCP::AUEP)
			("aucx", TMGCP::AUCX)
			("rsip", TMGCP::RSIP)
			;
	}
};


template <typename Iterator>
struct TmplMGCPparser : qi::grammar<Iterator>
{
	TmplMGCPparser(TMGCP& pack)
	: TmplMGCPparser::base_type(_MGCP, "MGCP"), m_MGCP(pack)
	{
#pragma region using
		using ascii::blank;
		using ascii::no_case;
		using qi::uint_;
		using qi::ushort_;
		using qi::char_;
		using qi::long_long;
		using qi::alpha;
		using qi::string;
		using qi::lit;
		using qi::lexeme;
		using qi::_val;
		using qi::_1;
		using qi::on_error;
		using qi::fail;
		using phoenix::ref;
		using phoenix::construct;
		using phoenix::val;
		using namespace qi::labels;
#pragma endregion using

#pragma region lambda functions
		auto setEndP_Addr = [&](const std::string& str){
			m_MGCP.EndPoint.m_addr = str;
		};
		auto setEndP_Pnt = [&](const std::string& str){
			m_MGCP.EndPoint.m_point = str;
		};
		auto setSDP_Name = [&](const std::string& str){
			m_MGCP.cllSDPs.back()->name = str;
		};
		auto addSDP = [&](){
			m_MGCP.cllSDPs.push_back(SHP_TSDP(new TSDP));
		};
		auto newMGCPParam =
			[&](const fusion::vector<TMGCP_Param::ParamType, std::string>& val)
		{
			m_MGCP.cllParams.insert({ fusion::at_c<0>(val), fusion::at_c<1>(val) });
		};
		auto addSDPParam =
			[&](const boost::fusion::vector<char, std::string>& val)
		{
			auto shpSDP = m_MGCP.cllSDPs.back();
			SHP_TSDP_Param shp(new TSDP_Param);
			shp->m_type = fusion::at_c<0>(val);;
			shp->m_value = fusion::at_c<1>(val);
			shpSDP->cllParams.push_back(shp);
		};
		
		auto setSDP_connection =
			[&](const boost::fusion::vector<std::string, std::string, std::string>& val)
		{
			auto shpSDP = m_MGCP.cllSDPs.back();
			TSDP::connection_data& data = shpSDP->connection;
			data.nettype	= fusion::at_c<0>(val);
			data.addrtype	= fusion::at_c<1>(val);
			data.address	= fusion::at_c<2>(val);
		};
		auto addSDP_timing = [&](const boost::fusion::vector<unsigned, unsigned>& val)
		{
			auto shpSDP = m_MGCP.cllSDPs.back();
			TSDP::timing tmp = { fusion::at_c<0>(val), fusion::at_c<1>(val) };
			m_MGCP.cllSDPs.back()->cllTiming.push_back(tmp);
		};
		auto setSDP_origin =
			[&](const boost::fusion::vector<std::string, unsigned long long, unsigned long long,
			std::string, std::string, std::string>& val)
		{
			auto shpSDP = m_MGCP.cllSDPs.back();
			TSDP& sdp = *shpSDP;
			sdp.org.username= fusion::at_c<0>(val);
			sdp.org.sess_id	= fusion::at_c<1>(val); 
			sdp.org.sess_ver= fusion::at_c<2>(val); 
			sdp.org.nettype	= fusion::at_c<3>(val);
			sdp.org.addrtype= fusion::at_c<4>(val);
			sdp.org.addr	= fusion::at_c<5>(val);
		};
		auto addMedia =
			[&](const boost::fusion::vector<std::string, unsigned short>& val)
		{
			m_MGCP.cllSDPs.back()->cllMedia.push_back
				({fusion::at_c<0>(val), fusion::at_c<1>(val)});
		};
		auto setMediaProto = [&](const std::string& str){
			m_MGCP.cllSDPs.back()->cllMedia.back().proto = str;			
		};
		auto setAttr_ptime = [&](unsigned short val){
			m_MGCP.cllSDPs.back()->cllMedia.back().ptime = val; };
		auto setAttr_maxptime = [&](unsigned short val){
			m_MGCP.cllSDPs.back()->cllMedia.back().maxptime = val; };
		auto addAttr = [&](const std::string& str){
			auto pmode = Mediamode_symb::instance().find(str);
			TSDP::media& media = m_MGCP.cllSDPs.back()->cllMedia.back();
			if (pmode)
				media.mode = *pmode;
			else
				media.cllAttrs.push_back(str);
		};
#pragma endregion lambda functions

		_return = -char_('\r') >> char_('\n');
		_strLine %= *(char_ - _return);
		_trimLStrLine = +blank >> _strLine;
		_word = -char_(' ') >> +(char_ - blank - _return)[_val += _1];

#pragma region MGCP_parsing
		_MGCP_Param		= _return >> (MGCP_Param_symb::instance() >> ':' >> _trimLStrLine)[newMGCPParam];
		_MGCP_ParMode	= _return >> "M: "  >> Conn_Mode_symb::instance()[ref(m_MGCP.parMode) = _1];
		_MGCP_ParConnId = _return >> "I: "  >> uint_[ref(m_MGCP.idConn) = _1];

		_MGCP_version = no_case["mgcp"] >> +blank
			>> uint_[ref(m_MGCP.verMajor) = _1] >> '.' >> uint_[ref(m_MGCP.verMinor) = _1];
		_EndP_Addr =
			//(('[' >> +(char_ - ']') >> ']')[_val += _1]) ||
			(+(char_ - blank)[_val += _1]);
		_EndP_Pnt %= +(char_ - ('@' || blank));// [_val += _1];

		_MGCP_header = qi::eps > (no_case[MGCP_Cmd_symb::instance()])[ref(m_MGCP.CMD) = _1]
			>> +blank >> uint_[ref(m_MGCP.IdTransact) = _1]
			>> +blank > _EndP_Pnt[setEndP_Pnt] >> '@' > _EndP_Addr[setEndP_Addr]
			>> +blank > _MGCP_version
			>> -(+blank >> _strLine)
			;
#pragma endregion MGCP_parsing

#pragma region SDP_parsing
		_sdp_origin = _return >> "o=" >>
			(_word >> ' ' >> long_long >> ' ' >> long_long >> _word >> _word >> _word)
			[setSDP_origin];
		_sdp_Name	= _return >> "s=" >> _strLine;
		_sdp_timing = _return >> "t=" >> ( uint_ >> ' ' >> uint_)[addSDP_timing];


		_mediaAttr = 
			(
				("ptime:" >> ushort_[setAttr_ptime])
				|| ("maxptime:" >> ushort_[setAttr_maxptime]) 
			)
			>> *blank
			;

		_sdpLevelMedia = _return >> "m=" 
			>> (_word >> ' ' >> ushort_)[addMedia] >> -('/' >> ushort_)
			>> _strLine[setMediaProto]/*_word[setMediaProto] >> (' ' >> ushort_)*/
			>> *(_return >> "a=" >> (_mediaAttr | _strLine[addAttr]/*not parsed attr*/));

		_sdp_connection = _return >> "c=" >> (_word >> _word >> ' ' >> _strLine)[setSDP_connection];
		_sdpLevelSession = qi::eps 
			> _sdp_origin
			> _sdp_Name[setSDP_Name]
			> _sdp_connection
			> +_sdp_timing
					;

		_SDP_Param = _return >> (alpha >> '=' >> _strLine)[addSDPParam];
		_SDP_Session = _return >> _return >> no_case["v=0"] >> qi::eps[addSDP]
			>>  _sdpLevelSession
			>> +_sdpLevelMedia
			>> *_SDP_Param //not parsed params
			;
#pragma endregion SDP_parsing

		_MGCP = qi::eps > _MGCP_header
			>> *(_MGCP_ParMode || _MGCP_ParConnId || _MGCP_Param)
			>> *_SDP_Session
			;

#pragma region error_handling
		_MGCP.name("MGCP");
		_MGCP_header.name("MGCP_header");
		_EndP_Pnt.name("EndP_Pnt");
		_EndP_Addr.name("EndP_Addr");
		_MGCP_Param.name("MGCP_Param");
		_MGCP_ParMode.name("MGCP_ParMode");
		_MGCP_ParConnId.name("MGCP_ParConnId");
		_SDP_Session.name("SDP");
		_sdpLevelSession.name("SDP Session Level");
		_sdp_origin.name("SDP origin (o=)");
		_sdp_connection.name("SDP connection (c=)");
		_sdp_Name.name("SDP Session name (s=)");
		_sdp_timing.name("SDP timing (t=)");
		_sdpLevelMedia.name("SDP Media Level");
		_SDP_Param.name("SDP Param");
		_strLine.name("text");
		_trimLStrLine.name("trimLStrLine");
		_return.name("\\n");

		on_error<fail>
			(
			_MGCP
			, std::cout
			<< val("============ Error in parsing! ============\nExpecting:\n")
			<< _4                               // what failed?
			<< val("\nhere:\n")
			<< construct<std::string>(_3, _2)   // iterators to error-pos, end
			<< val("\n==============\n")
			);
#pragma endregion error_handling
	}

private:
	TMGCP&		m_MGCP;

#pragma region rules
	qi::rule<Iterator> _MGCP;
	qi::rule<Iterator> _MGCP_header;
	qi::rule<Iterator> _MGCP_version;
	qi::rule<Iterator> _MGCP_Param;
	qi::rule<Iterator> _MGCP_ParMode;
	qi::rule<Iterator> _MGCP_ParConnId;
	qi::rule<Iterator, SHP_TSDP_Param> _SDP_Session;
	qi::rule<Iterator> _sdpLevelSession, _sdpLevelMedia;
	qi::rule<Iterator> _SDP_Param;
	qi::rule<Iterator> _sdp_origin;
	qi::rule<Iterator> _sdp_connection;
	qi::rule<Iterator> _sdp_timing;

	qi::rule<Iterator, std::string()> _mediaAttr;
	qi::rule<Iterator, std::string()> _EndP_Addr;
	qi::rule<Iterator, std::string()> _EndP_Pnt;
	qi::rule<Iterator, std::string()> _strLine;
	qi::rule<Iterator/*, std::string()*/> _return;
	qi::rule<Iterator, std::string()> _trimLStrLine;
	qi::rule<Iterator, std::string()> _word;
	qi::rule<Iterator, std::string()> _sdp_Name;
#pragma endregion rules
};


bool parseMGCP(const char* pCh, TMGCP& mgcp)
{
	TmplMGCPparser<const char*> parserMGCP(mgcp);
	auto pBegin = pCh;
	auto pEnd	= pCh + strlen(pCh);

	//bool b = parse(pBegin, pEnd, parserMGCP, ascii::space);
	bool b = phrase_parse(pBegin, pEnd, parserMGCP, ascii::space)
		&& pBegin == pEnd;
	printf("Parser %s\n", b ? "OK" : "false");


	return b;
}

}//namespace MGCPparser