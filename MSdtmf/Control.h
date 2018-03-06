#pragma once
#include "stdafx.h"
#include "Point.h"
namespace dtmf
{
	class Control
	{
	public:
		Control();
		void Preprocessing(SHP_IPL);

	private:
		void CR(SHP_IPL);
		void DL(SHP_IPL);

		void DeletePoint(SHP_Point);
		SHP_Point FindPoint(string);

		int GetFreeThread();

		void Receive(boost::system::error_code, size_t, SHP_Point);
		void RunIO(int);

		Data rawBuf;

		vector<SHP_IO> vecIOs;
		vector<int> vecIOstates;
		vector<SHP_Point> vecPoints;
	};
	typedef shared_ptr<Control> SHP_Control;
}