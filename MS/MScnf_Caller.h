#pragma once
#include "SL_All.h"


namespace cnf
{
	class Caller
	{
	public:
		Caller(string);
		~Caller();

		void SetJitterSize(int);
		void StoreFrame(SHP_FRAME, int);
		SHP_FRAME GetFrame(int);

		int ptime = 0; /* *10 */
		AVCodecContext* iccx;
		AVCodecContext* occx;

		SHP_Point basePoint;

	private:
		void InitCodec(AVCodecContext**, bool);//true-decoder, false-encoder

		JITTER bufFrame;

	};
	typedef shared_ptr<Caller> SHP_Caller;
}