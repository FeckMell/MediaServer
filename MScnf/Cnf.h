#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "CnfPoint.h"
#include "Audio.h"
using namespace std;

extern SHP_IPar init_Params;
//extern boost::asio::io_service io_Server;
extern boost::asio::io_service io_Apps;
//extern SHP_Socket outer_Socket;
extern SHP_Socket inner_Socket;


//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
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