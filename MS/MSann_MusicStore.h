#pragma once
#include "stdafx.h"
#include "SL_ALL.h"


namespace ann
{
	class MediaFile
	{
	public:
		MediaFile(string);

		SHP_PACKET GetPacket(int);
		int Size();

		string fileName;
		string error = "";

	private:

		int OpenFile();
		void CutPackets(vector<SHP_PACKET>);

		vector<SHP_PACKET> data;

	};

	typedef shared_ptr<MediaFile> SHP_MediaFile;
	//*///------------------------------------------------------------------------------------------
	//*///------------------------------------------------------------------------------------------
	class MusicStore
	{
	public:

		static SHP_MediaFile GetFile(string);

	private:

		static SHP_MediaFile OpenNewFile(string);

		static vector<SHP_MediaFile> data;

	};
	//typedef shared_ptr<MusicStore> SHP_MusicStore;
}