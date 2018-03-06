#pragma once
#include "stdafx.h"
#include "SharedLib.h"


namespace cnf
{
	class Point
	{
	public:
		Point(string, string, string, string);
		~Point();

		void SetJitterSize(int);
		void StoreFrame(SHP_FRAME, int);
		SHP_FRAME GetFrame(int);

		int ptime = 0; /* *10 */
		string clientPort;
		string clientIP;
		string serverPort;
		string serverIP;
		AVCodecContext* iccx;
		AVCodecContext* occx;
		SHP_SOCK socket;

	private:
		void InitCodec(AVCodecContext**, bool);//true-decoder, false-encoder

		JITTER bufFrame;

	};
	typedef shared_ptr<Point> SHP_Point;
}