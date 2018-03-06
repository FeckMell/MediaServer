#pragma once
#include "stdafx.h"
#include "MGCPparser.h"
#include "Point.h"
namespace mgcp
{
	class Ann
	{
	public:

		Ann(SHP_Point, SHP_MGCP);

		void RequestMusic(SHP_MGCP);
		void Delete();

		string eventID;

	private:

		bool CheckFileExistance();
		void SendToAnnModul(string);

		bool state = false;
		string fileName;
		SHP_Point point;
	};
	typedef shared_ptr<Ann> SHP_Ann;
}