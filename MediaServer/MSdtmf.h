#pragma once
#include "stdafx.h"
#include "SharedLib.h"
#include "MSdtmf_Point.h"


namespace dtmf
{
	class Control
	{
	public:
		Control();
		void Preprocessing(string);

	private:
		void CR(SHP_IPL);
		void DL(SHP_IPL);

		void RemovePoint(SHP_Point);
		SHP_Point FindPoint(string);

		vector<SHP_Point> vecPoints;
	};
	typedef shared_ptr<Control> SHP_Control;
}