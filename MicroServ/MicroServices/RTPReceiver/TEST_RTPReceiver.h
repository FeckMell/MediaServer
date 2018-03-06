#pragma once
#include "../../SharedLib.h"
#include <random>

namespace RTPReceiver
{
	class TO
	{
	public:
		TO(string c_port_, string s_port_, string id_, SHP_IO io_);

		string Param(string name_);
		SHP_IPL GetBasicIPL();
		void Send();
		void Func1(boost::any param_);

	private:


		SHP_SockBuf thisBuf;
		map<string, string> mapData;
		SHP_Socket thisSocket;

	}; typedef shared_ptr<TO> SHP_TO;

	class TESTRECEIVER
	{
	public:

		static void Init(int size_);
		static void Start( int no_requests_);

	private:

		static void SendAll();

		static SHP_thread sendThread;
		static SHP_IO toIO;
		static map<int, SHP_TO> mapTO;

	};
}