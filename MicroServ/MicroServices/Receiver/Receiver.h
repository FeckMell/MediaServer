#pragma once
#include "../../SharedLib.h"
#include "Receiver_Inst.h"

namespace Receiver
{
	class MainFrame
	{
	public:

		static void Init();
		static void DeInit();
		static void Proceed(boost::any param_);

	private:

		static void AddReceiver(SHP_IPL ipl_);
		static void DelReceiver(SHP_IPL ipl_);

		static void FakeReceive(boost::system::error_code ec_, size_t size_);

		static SHP_IO ioForAll; /*Caller Receive Thread*/
		static SHP_Socket fakeSocket;
		static SHP_thread receiveThread;

		static map<string, SHP_Inst> mapInst;

		static string modulName;

	};
}