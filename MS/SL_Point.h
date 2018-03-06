#include "stdafx.h"
#include "SL_SDP.h"
#include "SL_Socket.h"
#include "SL_LOG.h"

class Point
{
public:

	Point(string, SHP_SDP);

	SHP_SDP GetSDP(string);
	string GetParam(string);
	void ModifyClientSDP(SHP_SDP);
	string GetFullInfo();

	TIME2 timer;
	SHP_SOCK socket;

private:

	SHP_SDP serverSDP;
	SHP_SDP clientSDP;
	string callID;

	string state = "empty";
	string error = "";

};
typedef shared_ptr<Point> SHP_Point;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class POINTSTORE
{
public:

	static SHP_Point GetPoint(string);
	static SHP_Point CreatePoint(string, SHP_SDP);
	static void DeletePoint(string);

private:

	static map<string, SHP_Point> mapPoints;

};