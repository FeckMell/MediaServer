#include "stdafx.h"
#include "PackMGCP.h"

void tmp()
{
/*
	boost::asio::ip::address_v4 ip;
	//using namespace CPackMGCPParam::MGCPParam;
	auto val = CPackMGCPParam::BearerInformation;*/
}


namespace MGCP
{
	class TParamSymbMap : public std::map<TMGCP_Param::ParamType, const char*>
	{
	public:
		static const TParamSymbMap& Instance()
		{
			static TParamSymbMap single;
			return single;
		}
	private:
		TParamSymbMap(){
			insert({TMGCP_Param::BearerInformation, "B"});
			insert({TMGCP_Param::CallId,			"C"});
			insert({TMGCP_Param::Capabilities,		"A"});
			insert({TMGCP_Param::ConnectionId,		"I"});
			insert({TMGCP_Param::ConnectionMode,	"M"});
			insert({TMGCP_Param::ConnectionParameters, "P"});
			insert({TMGCP_Param::DetectEvents,		"T"});
			insert({TMGCP_Param::DigitMap,			"D"});
			insert({TMGCP_Param::EventStates,		"ES"});
			insert({TMGCP_Param::LocalConnectionOptions, "L"});
			insert({TMGCP_Param::MaxMGCPDatagram,	"MD"});
			insert({TMGCP_Param::NotifiedEntity,	"N"});
			insert({TMGCP_Param::ObservedEvents,	"O"});
			insert({TMGCP_Param::PackageList,		"PL"});
			insert({TMGCP_Param::QuarantineHandling,"Q"});
			insert({TMGCP_Param::ReasonCode,		"E"});
			insert({TMGCP_Param::RequestedEvents,	"R"});
			insert({TMGCP_Param::RequestedInfo,		"F"});
			insert({TMGCP_Param::RequestIdentifier, "X"});
			insert({TMGCP_Param::ResponseAck,		"K"});
			insert({TMGCP_Param::RestartDelay,		"RD"});
			insert({TMGCP_Param::RestartMethod,		"RM"});
			insert({TMGCP_Param::SecondConnectionId,"I2"});
			insert({TMGCP_Param::SecondEndpointId,	"Z2"});
			insert({TMGCP_Param::SignalRequests,	"S"});
			insert({TMGCP_Param::SpecificEndPointId,"Z"});
		}
		TParamSymbMap(TParamSymbMap&) = delete;
		TParamSymbMap& operator=(const TParamSymbMap&) = delete;
	};

	const char* TMGCP_Param::ParamName(ParamType t)
	{
		static const char* pEmpty = "";

		auto& map =TParamSymbMap::Instance();
		auto itr = map.find(t);
		if (itr != map.cbegin())
			return itr->second;

		assert(false);
		return pEmpty;
	}

	const char* TMGCP_Param::name() const
	{
		return TMGCP_Param::ParamName(m_type);
	}
	string TMGCP::getsdpparam()
	{
		//////
		return "";
	}
	string TMGCP::getCallID()
	{
		/*string strResult();
		for (const auto& param : cllParams)
			strResult += str(param.m_value);*/
		cllParams;
		static auto frmHeader = boost::format("");
		static auto frmParam = boost::format("%1%");

		string strResult(str(frmHeader));
		for (const auto& param : cllParams)
		if (param.name()=="C")
				strResult += str(frmParam % param.m_value);
		return strResult;
	}
	string TMGCP::ResponseBAD(unsigned short code) const
	{
		static auto frmHeader = boost::format("%1% %2% BAD");
		static auto frmParam = boost::format("\n%1%: %2%");

		string strResult(str(frmHeader % code % IdTransact));
		for (const auto& param : cllResponseParams)
			strResult += str(frmParam % param->name() % param->m_value);

		return strResult;
	}
	string TMGCP::ResponseOK(unsigned short code /*=200*/) const
	{
		static auto frmHeader = boost::format("%1% %2% OK");
		static auto frmParam  = boost::format("\n%1%: %2%");

		string strResult(str(frmHeader % code % IdTransact));
		for (const auto& param : cllResponseParams)
			strResult += str(frmParam % param->name() % param->m_value);

		return strResult;
	}

	SHP_TMGCP_Param TMGCP::getSignalParam()
	{
		//TODO: fucked shared
		SHP_TMGCP_Param result;

		for (auto& param : cllParams)
		{
			if (param.m_type == TMGCP_Param::SignalRequests)
				result = std::make_shared<TMGCP_Param>(param.m_type, param.m_value );
		}
		return result;
	}

	std::ostream& operator<<(std::ostream& os, const TSDP::connection_data& c)
	{
		return os << "\nc=" 
			<< c.nettype	<< ' ' 
			<< c.addrtype	<< ' ' 
			<< c.address;
	}
	std::ostream& operator<<(std::ostream& os, const TSDP::origin& org)
	{
		return os << "\no="
			<< org.username	<< ' '
			<< org.sess_id	<< ' '
			<< org.sess_ver	<< ' '
			<< org.nettype	<< ' '
			<< org.addrtype	<< ' '
			<< org.addr;
	}
	std::ostream& operator<<(std::ostream& os, const std::vector<TSDP::timing>& cllTiming)
	{
		for (auto& entry : cllTiming)
			os << "\nt=" << entry.start << ' ' << entry.stop;
		return os;
	};
	std::ostream& operator<<(std::ostream& os, const std::vector<TSDP::media>& cllMedia)
	{
		for (auto& entry : cllMedia)
		{
			/*
			//-------------SDP-------------
			v=0
			o=- 0 0 IN IP4 127.0.0.1
			s=No Name
			c=IN IP4 10.77.7.19
			t=0 0
			a=tool:libavformat 57.3.100
			m=audio 5004 RTP/AVP 14
			b=AS:96
			*/
			os << "\nm=" << entry.type << ' ' << entry.port << /*' ' <<*/ entry.proto
			/*	<< " 96\nb=AS:64"
				<< "\na=tool:libavformat 57.3.100"
				<< "\na=recvonly"*/;
		}
		return os;
	};

	string TSDP::toString() const
	{
		static auto frm = boost::format("\n\nv=0%1%\ns=%2%%3%%4%%5%");
		return str(frm
			% org 
			% name 
			% connection 
			% cllTiming 
			% cllMedia
			);
	}


	string TEndPoint::toString() const
	{
		//printf("\n test333\n");
		return str(boost::format("%1%@%2%") % m_point % m_addr);
	}

}