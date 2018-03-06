#pragma once
#include "stdafx.h"
#include "MusicStore.h"

extern SHP_STARTUP init_Params;

class Ann
{
public:
	Ann(SHP_MediaFile, SHP_IPL);
	void DL();

	string annID;
private:
	void Run();
	SHP_PACKET CreatePacket(int);
	void SendPacket(SHP_PACKET);

	bool state = true;
	SHP_thread th;

	IO ioAnn;
	SHP_SOCK outerSOCK;
	RTP_struct rtpHDR;
	SHP_MediaFile mediaFile;
	EP endPoint;
};
typedef shared_ptr<Ann> SHP_Ann;