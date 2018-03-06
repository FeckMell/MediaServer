#pragma once
#include "stdafx.h"
#include "SharedLib.h"
#include "MScnf_Point.h"
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
		void CreatePoints(vector<vector<string>>);

		vector<string> ParseLine(string);

		SHP_Audio mixerAudio;
		vector<SHP_Point> vecPoints;
	};
	typedef shared_ptr<Cnf> SHP_Cnf;
}