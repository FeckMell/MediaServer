#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "MusicStore.h"
using namespace std;
using boost::asio::ip::udp;

extern SHP_IPar init_Params;
//extern boost::asio::io_service io_Server;
extern boost::asio::io_service io_Apps;
//extern SHP_Socket outer_Socket;
extern SHP_Socket inner_Socket;

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