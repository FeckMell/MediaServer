#pragma once
#include "../SharedSource/stdafx.h"
#include "../SharedSource/Structs.h"
#include "../SharedSource/Functions.h"
#include "MusicStore.h"
#include "InnerParser.h"
using namespace std;
using boost::asio::ip::udp;

extern SHP_IPar init_Params;

class Ann
{
public:
	Ann(SHP_MediaFile, SHP_IPL);
	void DL();

	string annID;
private:
	void Run();
	SHP_CAVPacket CreatePacket(int);
	void SendPacket(SHP_CAVPacket);

	bool state = true;
	SHP_thread th;

	boost::asio::io_service ioAnn;
	SHP_Socket outerSocket;
	RTP_struct rtpHDR;
	SHP_MediaFile mediaFile;
	udp::endpoint endPoint;
};
typedef shared_ptr<Ann> SHP_Ann;