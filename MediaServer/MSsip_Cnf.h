#pragma once
#include "stdafx.h"
#include "SharedLib.h"
#include "MSsip_Point.h"


namespace sip
{
	class Cnf
	{
	public:

		Cnf(SHP_Point);

		void AddPoint(SHP_Point);
		bool RmPoint(SHP_Point);

		string eventID;

	private:
		void Process();
		void SendCR(vector<SHP_Point>);
		void SendDL();

		bool state = false;
		vector<SHP_Point> vecPoints;

	};
	typedef shared_ptr<Cnf> SHP_Cnf;
}