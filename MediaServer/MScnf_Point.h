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

		void SetMaxTimesTook(int);
		SHP_FRAME GetFrame();
		void StoreFrame(SHP_FRAME);

		string clientPort;
		string clientIP;
		string serverPort;
		string serverIP;
		AVCodecContext* iccx;
		AVCodecContext* occx;
		SHP_SOCK socket;

	private:
		void InitCodec(AVCodecContext**, bool);//true-decoder, false-encoder

		int timesTookMax = 0;
		int timesTook = 0;
		SHP_FRAME bufFrame = nullptr;
	};
	typedef shared_ptr<Point> SHP_Point;
}