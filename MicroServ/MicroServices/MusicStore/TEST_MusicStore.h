#pragma once
#include "../../stdafx.h"
#include "../../SharedLib.h"
#include "MusicStore.h"
#include <random>

namespace MusicStore
{
	class TO
	{
	public:

		TO(string filename_);

		void GetFile();
		void DelFile();
		void CheckFile();

	private:

		SHP_IPL BasicIPL();

		string fileName;

	}; typedef shared_ptr<TO> SHP_TO;
	//*///------------------------------------------------------------------------------------------
	//*///------------------------------------------------------------------------------------------
	//*///------------------------------------------------------------------------------------------
	//*///------------------------------------------------------------------------------------------
	class TESTMUSICSTORE
	{
	public:

		static void Init(map<string, string> params_);
		static void Start();
		static void CheckSavedFiles();

		static void Proceed(boost::any param_);

	private:

		static void InitUsual();
		static void InitTO();
		static void StartThread();

		static void ProceedResponse(boost::any param_);
		static void SaveFile(SHP_IPL ipl_);

		static int Random(int extra_seed_, int begin_, int end);
		static void FakeReceive(boost::system::error_code ec_, size_t size_);
		static SHP_IO ioThis;
		static SHP_Socket fakeSocket;
		static SHP_thread receiveThread;

		static string modulNameThis;
		static string modulNameTest;
		static map<int, SHP_TO> mapTO;
		static map<string, string> mapData;
		static vector<SHP_VecSockBuf> vecSavedFiles;

	};
}