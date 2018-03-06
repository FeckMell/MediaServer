#pragma once
#include "stdafx.h"
extern BOOSTLOGGER lg;


struct STARTUP
{
	//Tipes of parametrs.
	enum ParamNames
	{
		outerIP, innerIP, innerPort, logLevel, outerPort, rtpPort, maxTimeAnn, maxTimeCnf, maxTimePrx,
		mediaPath, homePath,
		maxParamNames
	};
	//Methods for Data.
	STARTUP(char**, string);
	string GetParams();

	//Data.
	string modulName;
	vector<string> data;
	string error = "";
};
typedef shared_ptr<STARTUP> SHP_STARTUP;
extern SHP_STARTUP init_Params;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class SOCK
{
public:
	SOCK(string, int, IO&);
	~SOCK();
	boost::asio::ip::udp::socket s;
};
typedef shared_ptr<SOCK> SHP_SOCK;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
struct REQUEST
{
	EP sender;
	char data[2048];
};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class PACKET
{
public:
	PACKET();
	PACKET(size_t sz);
	~PACKET();

	AVPacket* Get();
	int Size();
	uint8_t* Data();
	void Free();
	void MakeSize(int n);
private:
	AVPacket packet;
};
typedef shared_ptr<PACKET> SHP_PACKET;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class NETDATA
{
public:
	enum E{ main, ser, ann, cnf, prx, sip, maxE };
	enum S{ out, in, maxS };

	NETDATA(int);
	SHP_SOCK GS(int);//GetSOCK
	EP GE(int);//GetEndpPoint
	IO& GI(int);//GetIO
	void SendModul(int, string);

private:
	vector<SHP_SOCK> sockets;
	vector<EP> endPoints;
	vector<SHP_IO> ios;
};
typedef shared_ptr<NETDATA> SHP_NETDATA;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
// R T P -> R T PMES
struct RTP
{
	/* первый байт */
	uint8_t csrc_len : 4;//
	uint8_t extension : 1;//
	uint8_t padding : 1;//
	uint8_t version : 2;//
	/* второй байт */
	uint8_t payload_type : 7;//
	uint8_t marker : 1;//
	/* третий-четвертый байты */
	uint16_t seq_no;
	/* пятый-восьмой байты */
	uint32_t timestamp;//
	/* девятый-двенадцатый байт */
	uint32_t ssrc;//
};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//R T P_struct -> GAVSRT111 -> R T P 
struct RTP_struct
{
	RTP header;
	long TS;
	int amount;

	RTP_struct();
	void Config();
	RTP Get();
};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
struct Data
{
	uint8_t data[1000];
	int size = 160;
};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class FRAME
{
public:
	FRAME();
	FRAME(bool a);
	~FRAME();

	AVFrame* Get();
	void Free();
	bool Empty();
private:
	bool empty;
	AVFrame* frame;
};
typedef shared_ptr<FRAME> SHP_FRAME;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
struct FFF
{
	~FFF()
	{
		for (auto &e : sinkVec) avfilter_free(e);//?
		for (auto &e1 : afcx) for (auto &e2 : e1) avfilter_free(e2);//??
		for (auto &e : graphVec) avfilter_graph_free(&e);//!
	}
	std::vector<vector<AVFilterContext*>> afcx;
	std::vector<AVFilterGraph*> graphVec;
	std::vector<AVFilterContext*> sinkVec;
};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------

