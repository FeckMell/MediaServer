#pragma once
#include "../../stdafx.h"
#include "../../SharedLib.h"
#include "MusicStore_MusicFile.h"
namespace MusicStore
{
	class MainFrame
	{
	public:

		static void Init();
		static void DeInit();
		static void Proceed(boost::any param_);

	private:

		static void GetFile(SHP_IPL ipl_);
		static void DelFile(SHP_IPL ipl_);
		static void CheckFile(SHP_IPL ipl_);

		static SHP_IPL BasicIPL();
		static SHP_MusicFile OpenNewFile(SHP_IPL ipl_);

		static void FakeReceive(boost::system::error_code ec_, size_t size_);

		static SHP_IO ioMainFrame;
		static SHP_Socket fakeSocket;
		static SHP_thread receiveThread;

		static map<string, SHP_MusicFile> mapMusicFile;

		static string modulName;
	};

}