#pragma once
#include "SL_All.h"
#include "MScnf_Caller.h"
#include "MScnf_Filter.h"


namespace cnf
{
	class Audio
	{
	public:

		Audio(vector<SHP_Caller>);
		~Audio();

	private:
		/*initing and reiniting*/
		void CreateSilentFrame();
		void Run();
		void RunIO();

		/*Main activity*/
		void Receive(boost::system::error_code, size_t, int);
		void PrepareData(SHP_PACKET, int);
		SHP_FRAME Decode(SHP_PACKET, int);
		void ProceedData(SHP_PACKET, int);
		void FillFilter(int);
		void EncodeAndSend(int);
		SHP_FRAME GetFrameFromFilter(int);
		void SaveFrameToJitters(SHP_FRAME, int);

		/*Data*/
		SHP_FRAME silentFrame;
		SHP_thread eventThread;
		bool state;
		SHP_Filter filter;
		vector<SHP_Caller> vecCallers;
	};
	typedef shared_ptr<Audio> SHP_Audio;
}