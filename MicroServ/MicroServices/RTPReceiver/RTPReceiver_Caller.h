#pragma once
#include "../../SharedLib.h"

namespace RTPReceiver
{
	class Caller; typedef shared_ptr<Caller> SHP_Caller;
	class Caller
	{
	public:
		Caller(SHP_IPL ipl_, SHP_IO io_);
		~Caller();

		string Param(string name_);
		void Stop();
		void AddFunc(SHP_IPL ipl_);
		void DelFunc(SHP_IPL ipl_);

		void TestSafe(SHP_Caller c_);

	private:

		void Receive(boost::system::error_code ec_, size_t size_);

		map<string, string> mapData;
		map<string, boost::function<void(boost::any)>> mapFunc;

		SHP_Socket thisSocket;
		SHP_Caller thisCaller;


	}; 
}