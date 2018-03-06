#pragma once
#include "../../stdafx.h"
#include "../../SharedLib.h"
#include "OverLoad_Caller.h"
namespace OverLoad
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

		static void CheckOverLoad(string c_ip_, string c_port_);
		static void ReportCollision(SHP_Caller caller_);

		static void FakeReceive(boost::system::error_code ec_, size_t size_);

		static SHP_IO ioMainFrame; 
		static SHP_Socket fakeSocket;
		static SHP_thread receiveThread;

		static map<string, SHP_Caller> mapCaller;

		static string modulName;
	};

}