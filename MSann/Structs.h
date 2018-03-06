#pragma once
#include "stdafx.h"
using namespace std;
extern src::severity_logger<severity_level> lg;

typedef shared_ptr<thread> SHP_thread;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
struct IPar
{
	//Tipes of parametrs.
	enum ParamNames
	{
		outerIP, innerIP, innerPort, logLevel, outerPort, rtpPort, maxTimeAnn, maxTimeCnf, maxTimePrx,
		mediaPath, homePath,
		maxParamNames
	};
	//Methods for Data.
	IPar(char** argv_);
	string GetParams();

	//Data.
	string modulName = "ann";
	vector<string> data;
	string error = "";
};
typedef shared_ptr<IPar> SHP_IPar;
extern SHP_IPar init_Params;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class Socket
{
public:
	Socket(string, int, boost::asio::io_service&);
	~Socket();
	boost::asio::ip::udp::socket s;
};
typedef shared_ptr<Socket> SHP_Socket;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
struct Request
{
	boost::asio::ip::udp::endpoint sender;
	char data[2048];
};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class CAVPacket
{
public:
	CAVPacket();
	CAVPacket(size_t sz);
	~CAVPacket();

	AVPacket* Get();
	int Size();
	uint8_t* Data();
	void Free();
	void MakeSize(int n);
private:
	AVPacket packet;
};
typedef shared_ptr<CAVPacket> SHP_CAVPacket;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
struct RTP
{
	// первый байт 
	uint8_t csrc_len : 4;//
	uint8_t extension : 1;//
	uint8_t padding : 1;//
	uint8_t version : 2;//
	// второй байт 
	uint8_t payload_type : 7;//
	uint8_t marker : 1;//
	// третий-четвертый байты 
	uint16_t seq_no;
	// пятый-восьмой байты 
	uint32_t timestamp;//
	// девятый-двенадцатый байт 
	uint32_t ssrc;//
};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
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
class NETDATA
{
public:
	enum E{ main, ser, ann, cnf, prx, sip, maxE };
	enum S{ out, in, maxS };

	NETDATA();
	SHP_Socket GS(int);//GetSocket
	EP GE(int);//GetEndpPoint
	IO& GI(int);//GetIO

private:
	vector<SHP_Socket> sockets;
	vector<EP> endPoints;
	vector<SHP_IO> ios;
};
typedef shared_ptr<NETDATA> SHP_NETDATA;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------