#pragma once
#include "SL_ALL.h"

namespace mgcp
{

	class Cnf
	{
	public:

		Cnf(SHP_Point, string, SHP_SDP);

		void AddPoint(SHP_Point);
		bool DeletePoint(SHP_Point);

		void Process();

		string eventID;

	private:

		void SendDL();
		void SendCR(vector<SHP_Point>);

		bool state = false;
		vector<SHP_Point> vecPoints;
	};
	typedef shared_ptr<Cnf> SHP_Cnf;

}