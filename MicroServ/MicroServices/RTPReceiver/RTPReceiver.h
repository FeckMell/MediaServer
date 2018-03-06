#pragma once
#include "../../SharedLib.h"
#include "RTPReceiver_Caller.h"

namespace RTPReceiver
{
	class MainFrame
	{
	public:

		static void Init();
		static void DeInit();
		static void Proceed(boost::any param_);

	private:

		static void AddCaller(SHP_IPL ipl_);
		static void DelCaller(SHP_IPL ipl_);

		static void AddFunc(SHP_IPL ipl_);
		static void DelFunc(SHP_IPL ipl_);

		static void FakeReceive(boost::system::error_code ec_, size_t size_);

		static SHP_IO ioForAll; /*Caller Receive Thread*/
		static SHP_Socket fakeSocket;
		static SHP_thread receiveThread;

		static map<string, SHP_Caller> mapCaller;

		static string modulName;

	};
}