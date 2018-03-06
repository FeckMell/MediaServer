#pragma once
#include "../../SharedLib/All.h"
#include "FAnn_Music.h"
namespace NFAnn
{
	class Point
	{
	public:
		Point(SHP_IPL ipl_, SHP_MusicFile file_);
		~Point();

		void SendNextPacket();

		string Param(string name_);
		string PrintAll();

	private:

		void Report(string type_);

		SHP_Caller baseCaller;

		SHP_MusicFile mFile;
		map<string, string> mapData;
		int fileProgress = 0;

	};
	typedef shared_ptr<Point> SHP_Point;
}