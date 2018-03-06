#pragma once
#include "../../SharedLib/All.h"
#include "FCnf_Point.h"
#include "FCnf_Filter.h"

namespace NFCnf
{
	class Mixer
	{
	public:

		Mixer(vector<SHP_Point> vecPoint);
		~Mixer();
		void StopActivity();

	private:

		void CreateSilentFrame();

		void PointReceiveAction(int i_, SHP_SockBuf sock_buf_);
		void ProceedData(int i_, SHP_SockBuf sock_buf_);
		SHP_FRAME Decode(SHP_PACKET packet_, int i_);
		void SaveFrameToJitters(SHP_FRAME frame_, int from_which_);
		void FillFilter(int i_);
		void EncodeAndSend(int i_);

		void FakeReceive(boost::system::error_code ec_, size_t size_);
		SHP_Socket fakeSocket;

		SHP_IO cnfIO;
		SHP_FRAME silentFrame;
		SHP_thread actionThread;
		SHP_Filter shpFilter;
		vector<SHP_Point> vecPoint;

		

	};
	typedef shared_ptr<Mixer> SHP_Mixer;
}