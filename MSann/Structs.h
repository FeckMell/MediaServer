#pragma once
#include "stdafx.h"
using namespace std;

typedef shared_ptr<thread> SHP_thread;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
struct IPar
{
	//Tipes of parametrs.
	enum ParamNames
	{
		modulName, innerIP, innerPort, outerIP, pathMedia, pathHome, logLevel, 
		maxParamNames
	};
	//Methods for Data.
	IPar(char* argv_[]);
	string GetParams();

	//Data.
	vector<string> data;
	string error = "";
};
typedef shared_ptr<IPar> SHP_IPar;
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