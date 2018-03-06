#pragma once
#include "stdafx.h"
#include "SL_ALL.h"
#include "MSsip_Caller.h"


namespace sip
{
	class Cnf
	{
	public:
		Cnf(SHP_Caller);

		void AddPoint(SHP_Caller);
		bool RmPoint(SHP_Caller);

		string eventID;

	private:
		void Process();
		void SendCR(vector<SHP_Caller>);
		void SendDL();

		bool state = false;
		vector<SHP_Caller> vecPoints;

	};
	typedef shared_ptr<Cnf> SHP_Cnf;
}