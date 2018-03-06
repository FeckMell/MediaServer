#pragma once
#include "../SharedSource/stdafx.h"
#include "../SharedSource/Structs.h"
#include "../SharedSource/Functions.h"
#include "../SharedSource/InnerParser.h"
#include "MusicStore.h"

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

	IO ioAnn;
	SHP_Socket outerSocket;
	RTP_struct rtpHDR;
	SHP_MediaFile mediaFile;
	EP endPoint;
};
typedef shared_ptr<Ann> SHP_Ann;