#pragma once
#include "../../SharedLib/All.h"

namespace NFCnf
{
	class Point
	{
	public:
		Point(string call_id_);
		~Point();

		void SetJitterSize(int size_);
		void StoreFrame(SHP_FRAME frame_, int where_);
		SHP_FRAME GetFrame(int which_);

		int ptime = 0; /* *10 */
		AVCodecContext* iccx;
		AVCodecContext* occx;

		SHP_Caller baseCaller;

	private:

		void InitCodec(AVCodecContext** xccx_, bool type_);//true-decoder, false-encoder

		
		Jitter thisJitter;

	};
	typedef shared_ptr<Point> SHP_Point;
}