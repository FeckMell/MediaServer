#pragma once
#include "stdafx.h"
#include "SharedLib.h"
#include "MSmgcp_Point.h"


namespace mgcp
{
	class Cnf
	{
	public:
		Cnf(SHP_Point, string);
		void AddPoint(SHP_Point);
		bool DeletePoint(SHP_Point);
		void Process(SHP_Point);

		string eventID;
	private:
		int GetNumOfActivePoints();
		bool state = false;
		void SendCnfModul(string);
		vector<SHP_Point> vecPoints;
	};
	typedef shared_ptr<Cnf> SHP_Cnf;
}