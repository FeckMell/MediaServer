#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "CnfPoint.h"
using namespace std;

extern SHP_IPar init_Params;
//extern boost::asio::io_service io_Server;
extern boost::asio::io_service io_Apps;
//extern SHP_Socket outer_Socket;
extern SHP_Socket inner_Socket;


//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class Filter
{
public:
	Filter(vector<SHP_CnfPoint>);

	FFF data;

private:
	/*main activity*/
	int InitFilterGraph(int);

	/*Data*/
	int tracks;
	vector<SHP_CnfPoint> cnfPoints;
};
typedef shared_ptr<Filter> SHP_Filter;