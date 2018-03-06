#pragma once
#include "SL_ALL.h"
#include "MSsip_Cnf.h"
#include "MSsip_Caller.h"


namespace sip
{
	class Control
	{
	public:
		Control();

		void PreprocessingINNER(string);
		void PreprocessingOUTTER();


	private:
		void Invite(SHP_SIP);
		void Bye(SHP_SIP);
		void Ack(SHP_SIP);

		void ConfProcess(SHP_Caller, bool);

		SHP_Caller FindCaller(string);
		void RemoveCaller(SHP_Caller);

		SHP_Cnf FindCnf(string);
		void RemoveCnf(SHP_Cnf);

		vector<SHP_Cnf> vecCnfs;
		vector<SHP_Caller> vecCallers;
	};
	typedef shared_ptr<Control> SHP_Control;
}