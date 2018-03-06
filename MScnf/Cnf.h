#pragma once
#include "../SharedSource/stdafx.h"
#include "../SharedSource/Structs.h"
#include "../SharedSource/Functions.h"
#include "../SharedSource/InnerParser.h"
#include "CnfPoint.h"
#include "Audio.h"

extern SHP_STARTUP init_Params;

class Cnf
{
public:
	Cnf(SHP_IPL);
	~Cnf();

	void MD(SHP_IPL);
	void DL(SHP_IPL);

	string cnfID;
private:
	void ParsePoints(SHP_IPL);
	void AddRmPoint(vector<string>);
	void CreatePoints(vector<vector<string>>);

	SHP_Audio mixerAudio;
	vector<SHP_CnfPoint> vecPoints;
	boost::asio::io_service ioCnf;
};
typedef shared_ptr<Cnf> SHP_Cnf;