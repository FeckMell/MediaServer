#pragma once
#include "stdafx.h"
#include "SharedLib.h"
#include "MSsip_SIPparser.h"
#include "MSsip_Point.h"
#include "MSsip_Cnf.h"


namespace sip
{
	class Control
	{
	public:
		Control();

		void PreprocessingIN(string);
		void PreprocessingOUT(REQUEST);


	private:
		void Invite(SHP_SIP);
		void Bye(SHP_SIP);
		void Ack(SHP_SIP);

		void ConfProcess(SHP_Point, bool);

		SHP_Point FindPoint(string);
		void RemovePoint(SHP_Point);
		SHP_Cnf FindCnf(string);
		void RemoveCnf(SHP_Cnf);

		vector<SHP_Point> vecPoints;
		vector<SHP_Cnf> vecCnfs;
	};
	typedef shared_ptr<Control> SHP_Control;
}