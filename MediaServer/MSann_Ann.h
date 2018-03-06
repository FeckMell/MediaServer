#pragma once
#include "stdafx.h"
#include "SharedLib.h"
#include "MSann_MusicStore.h"


namespace ann
{
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
		SHP_SOCK outerSOCK = nullptr;
		RTP_struct rtpHDR;
		SHP_MediaFile mediaFile;
		EP endPoint;
	};
	typedef shared_ptr<Ann> SHP_Ann;
}