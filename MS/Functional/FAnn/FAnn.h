#pragma once
#include "../../SharedLib/All.h"
#include "FAnn_Point.h"
#include "FAnn_Music.h"

namespace NFAnn
{
	class FAnn
	{
	public:
		
		static void Init();
		static void ProceedRequest(string request_);

	private:

		static void Create(SHP_IPL ipl_);
		static void Delete(SHP_IPL ipl_);

		static void SendAnnFunc();

		static void FakeReceive(boost::system::error_code ec_, size_t size_);

		static SHP_IO ioForAll; /* SendFunction io*/
		static SHP_Socket fakeSocket;
		static SHP_thread actionThread;

		static map<string, SHP_Point> mapPoint;
		static mutex dataMutex;

	};
}