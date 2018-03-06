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

		string eventID;
		string callID;

	private:

		void Run();
		SHP_PACKET CreatePacket(int);
		void SendPacket(SHP_PACKET);

		void SendModul(string);


		bool state = true;
		SHP_thread th;

		string loop;
		SHP_SOCK outerSOCK = nullptr;
		SHP_MediaFile mediaFile;
		EP endPoint;

	};
	typedef shared_ptr<Ann> SHP_Ann;
}