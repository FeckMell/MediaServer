#pragma once
#include "stdafx.h"
#include "SharedLib.h"
#include "MScnf_CnfPoint.h"
#include "MScnf_Filter.h"


namespace cnf
{
	class Audio
	{
	public:
		Audio(vector<SHP_CnfPoint>);
		~Audio();
		//void MD(vector<SHP_CnfPoint>);
		//void Stop();

	private:
		/*initing and reiniting*/
		void CreateSilentFrame();
		void Run();
		void RunIO();

		/*Main activity*/
		void Receive(boost::system::error_code, size_t, int);
		SHP_FRAME Decode(SHP_PACKET, int);
		void ProceedData(int);
		void FillFilter(int);
		void EncodeAndSend(SHP_FRAME, int);
		SHP_FRAME GetFrameFromFilter(int);

		/*Data*/
		SHP_FRAME silentFrame;
		SHP_thread eventThread;
		bool state;// = true;
		Data rawBuf;
		SHP_Filter filter;
		vector<SHP_CnfPoint> vecPoints;
	};
	typedef shared_ptr<Audio> SHP_Audio;
}