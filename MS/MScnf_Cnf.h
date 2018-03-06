#pragma once
#include "SL_All.h"
#include "MScnf_Caller.h"
#include "MScnf_Audio.h"


namespace cnf
{
	class Cnf
	{
	public:
		Cnf(SHP_IPL);
		~Cnf();

		string eventID;

	private:

		void ParsePoints(SHP_IPL);

		SHP_Audio mixerAudio;
		vector<SHP_Caller> vecCallers;
	};
	typedef shared_ptr<Cnf> SHP_Cnf;
}