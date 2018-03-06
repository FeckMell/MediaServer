#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
using namespace std;

extern SHP_InitParams init_Params;
extern boost::asio::io_service io_Server;
extern boost::asio::io_service io_Apps;
extern SHP_Socket outer_Socket;
extern SHP_Socket inner_Socket;

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class MGCP
{
public:
	/*Enum struct data*/
	enum ParamNames {
		ParamC, ParamL, ParamI, ParamZ, ParamS, EventNum, EventFull, MessNum, SDP,
		MaxParamNames
	};
	enum EventNames { CMD, Type, State, MaxEventName };
	enum EventCMD { CRCX, RQNT, MDCX, DLCX, MaxEventCMD };
	enum EventType { ann, cnf, prx, MaxEventType };
	enum EventStates { confrnce, inactive, sendrecv, MaxEventState };
	/*String names for enumed data*/
	static vector<string> paramNamesStr;
	static vector<string> eventNamesStr;
	static vector<string> eventTypeStr;
	static vector<string> eventCMDStr;
	static vector<string> eventStatesStr;
	
	/*Main public activity*/
	MGCP(char*, boost::asio::ip::udp::endpoint);
	string ResponseOK(int, string);
	string ResponseBAD(int, string);

	/*sub functions to make string names from enumed params and back*/
	string EnumToStr(int, int);
	int StrToEnum(string);

	/*Access to parsed data*/
	string PrintAll();

	/*Data storage*/
	vector<int> events;
	vector<string> data;
	string MGCPstring;
	boost::asio::ip::udp::endpoint sender;
	string error = "";

private:
	/*Parsing functions*/
	void Parse();
	void ParseRest();
	void Remove();
	void ParseCMDandType();
};
typedef shared_ptr<MGCP> SHP_MGCP;