#pragma once
#include "../../stdafx.h"
#include "../../SharedLib.h"
namespace MusicStore
{
	class MusicFile
	{
	public:

		MusicFile(SHP_IPL ipl_);

		string Param(string name_);
		SHP_VecSockBuf Data();

	private:

		int OpenFile();

		map<string, string> mapData;
		SHP_VecSockBuf dataThis = nullptr;


	}; typedef shared_ptr<MusicFile> SHP_MusicFile;
}