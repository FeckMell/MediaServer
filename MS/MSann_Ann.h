#pragma once
#include "stdafx.h"
#include "SL_ALL.h"
#include "MSann_MusicStore.h"


namespace ann
{
	class Ann
	{
	public:

		Ann(SHP_MediaFile, SHP_IPL);
		string GetParam(string);

		void DL();		

	private:

		void Run();
		SHP_PACKET CreatePacket(int);
		void SendPacket(SHP_PACKET);
		void SendModul(string);

		string loop;
		string eventID;
		bool state = true;
		SHP_thread th;
		
		SHP_MediaFile mediaFile;
		SHP_Point basePoint;

	};
	typedef shared_ptr<Ann> SHP_Ann;
}