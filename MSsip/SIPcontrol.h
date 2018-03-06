#pragma once
#include "stdafx.h"
#include "SIPparser.h"
#include "Point.h"
#include "Cnf.h"
namespace sip
{
	class SIPcontrol
	{
	public:
		SIPcontrol();

		void Preprocessing(SHP_SIP);
		void Preprocessing(SHP_IPL);

	private:
		void Invite(SHP_SIP);
		void Bye(SHP_SIP);
		void Ack(SHP_SIP);

		void ConfProcess(SHP_Point, bool);//

		SHP_Point FindPoint(string);
		void RemovePoint(SHP_Point);
		SHP_Cnf FindCnf(string);
		void RemoveCnf(SHP_Cnf);

		string GenSDP(string, SHP_SIP);

		string ReservePort();
		void FreePort(string);

		string ReserveEventID();
		void FreeEventID(string);

		vector<int> usedPorts;
		vector<int> usedEventID;
		int lastSDP_ID;

		vector<SHP_Point> vecPoints;
		vector<SHP_Cnf> vecCnfs;
	};
	typedef shared_ptr<SIPcontrol> SHP_SIPcontrol;
}