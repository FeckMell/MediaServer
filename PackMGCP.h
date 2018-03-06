#pragma once

namespace MGCP
{

	struct TEndPoint
	{
		string		m_point;
		string		m_addr;

		string toString() const;
		//boost::asio::ip::address_v4	m_ip;
		bool operator <(const TEndPoint& p)const{
			return m_point < p.m_point;
		}
	};

	struct TMGCP_Param
	{
		enum ParamType{
			NON,
			BearerInformation,	//B
			CallId,				//C
			Capabilities,		//A
			ConnectionId,		//I
			ConnectionMode,		//M
			ConnectionParameters, //P
			DetectEvents,		//T
			DigitMap,			//D
			EventStates,		//ES
			LocalConnectionOptions, //L
			MaxMGCPDatagram,	//MD
			NotifiedEntity,		//N
			ObservedEvents,		//O
			PackageList,		//PL
			QuarantineHandling, //Q
			ReasonCode,			//E
			RequestedEvents,	//R
			RequestedInfo,		//F
			RequestIdentifier,  //X
			ResponseAck,		//K
			RestartDelay,		//RD
			RestartMethod,		//RM
			SecondConnectionId, //I2
			SecondEndpointId,	//Z2
			SignalRequests,		//S
			SpecificEndPointId  //Z
		};
		TMGCP_Param() : m_type(NON){
			//int dummy = 0;
		}
		TMGCP_Param(ParamType t, const string& val)
			: m_type(t), m_value(val)
		{
			assert(t != NON);
		}
		static const char*  ParamName(ParamType t);
		const char*  name() const;
//		operator bool()const{ return m_type != NON; }
		bool operator<(const TMGCP_Param& par)const{ return m_type < par.m_type; }

		ParamType	m_type;
		string m_value;
	};
	typedef shared_ptr<TMGCP_Param> SHP_TMGCP_Param;

	struct TSDP_Param
	{
		char		m_type;
		string	m_value;
	};
	typedef shared_ptr<TSDP_Param> SHP_TSDP_Param;

	struct TSDP
	{
		struct media
		{
			enum mmode{
				inactive, recvonly, sendrecv, sendonly
			};

			media(const string& t, unsigned short p):
				type(t), port(p){}

			string		type;
			unsigned short	port;
			mmode mode= inactive;
			//unsigned short  numports=0;
			string		proto;
			unsigned short	ptime = 0;
			unsigned short	maxptime = 0;
			std::vector<string> cllAttrs;
		};
		struct timing
		{
			unsigned start, stop;
		};
		struct connection_data
		{
			string
				nettype,
				addrtype,
				address;
		};
		struct origin{
			unsigned long long
			sess_id,
			sess_ver;

			string 
				username,
				nettype,
				addrtype,
				addr;
		};
		origin						org;
		string					name;
		connection_data				connection;
		std::vector<SHP_TSDP_Param> cllParams;
		std::vector<timing>			cllTiming;
		std::vector<media>			cllMedia;

		string toString() const;
	};

	typedef shared_ptr<TSDP> SHP_TSDP;
	typedef std::vector<SHP_TSDP> CLL_SDPs;
	typedef std::vector<SHP_TMGCP_Param> CLL_MGCP_Params;

	struct TMGCP
	{
	public:
		TMGCP() : CMD(NON), IdTransact(0) {};
		enum ConnMode{ non, sendonly, recvonly, sendrecv, confrnce, inactive, 
			loopback, conttest, netwloop, netwtest};
		enum MGCPCmd { NON, EPCF, CRCX, MDCX, DLCX, RQNT, NTFY, AUEP, AUCX, RSIP };
		string ResponseOK(unsigned short code = 200) const;
		string ResponseBAD(unsigned short code) const;
		string getCallID(); // "вытаскивает" CALLID
		string getsdpparam();// "вытаскивает" из SDP

		MGCPCmd			CMD;
		unsigned		IdTransact;
		TEndPoint		EndPoint;
		unsigned		verMajor, verMinor;
		ConnMode		parMode = non;
		unsigned		idConn = 0;

		SHP_TMGCP_Param getSignalParam();

		std::set<TMGCP_Param> cllParams;
		CLL_MGCP_Params cllResponseParams;
		CLL_SDPs		cllSDPs;
		std::vector<string> ID;

		void addResponseParam(const TMGCP_Param& param)
		{
			cllResponseParams.push_back(std::make_shared<TMGCP_Param>(param));
		}
	};
	typedef shared_ptr<TMGCP> SHP_TMGCP;


}