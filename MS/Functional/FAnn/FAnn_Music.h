#pragma once
#include "../../SharedLib/All.h"
namespace NFAnn
{
	class MusicFile
	{
	public:

		MusicFile(string name_);

		string Param(string name_);

		SHP_PACKET GetPacket(int seq_);
		int Size();

	private:

		int OpenFile();
		void CutPackets(vector<SHP_PACKET> file_);

		vector<SHP_PACKET> vecPackets;
		map<string, string> mapData;

	};
	typedef shared_ptr<MusicFile> SHP_MusicFile;



	class MusicStore
	{
	public:

		static SHP_MusicFile GetFile(string name_);
		
	private:

		static SHP_MusicFile OpenNewFile(string name_);
		static vector<SHP_MusicFile> vecFile;

	};
}