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
		modulName, innerIP, innerPort, outerIP, pathHome, logLevel,
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
struct InnerMes
{
	EP sender;
	char rawMes[2048];
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
class CAVFrame
{
public:
	CAVFrame();
	CAVFrame(bool a);
	~CAVFrame();

	AVFrame* Get();
	void Free();
	bool Empty();
private:
	bool empty;
	AVFrame* frame;
};
typedef shared_ptr<CAVFrame> SHP_CAVFrame;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
struct FFF
{
	~FFF() 
	{
		//for (auto &e : sinkVec) avfilter_free(e);//?
		//for (auto &e1 : afcx) for (auto &e2 : e1) avfilter_free(e2);//??
		for (auto &e : graphVec) avfilter_graph_free(&e);//!
	}
	std::vector<vector<AVFilterContext*>> afcx;
	std::vector<AVFilterGraph*> graphVec;
	std::vector<AVFilterContext*> sinkVec;
};
