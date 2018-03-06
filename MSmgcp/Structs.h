#pragma once
#include "stdafx.h"
using namespace std;


//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
struct IPar
{
	//Tipes of parametrs.
	enum ParamNames 
	{ 
		RTPPort, outerPort, innerPort, MediaPath, outerIP, innerIP, MaxTimeAnn, MaxTimeCnf, MaxTimePrx,
		LogLevel, PathEXE,
		MaxParamNames
	};
	//Methods for Data.
	IPar(char*);
	string GetParams();
	void GetPathExe(char*);
	void ParseConfigFile();


	//Data.
	vector<string> data;
	string error = "";
};
typedef shared_ptr<IPar> SHP_IPar;
extern SHP_IPar init_Params;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class Socket
{
public:
	Socket(string, int, IO&);
	~Socket();
	boost::asio::ip::udp::socket s;
};
typedef shared_ptr<Socket> SHP_Socket;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
struct Request
{
	EP sender;
	char data[2048];
};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class NETDATA
{
public:
	enum E{ main, ser, ann, cnf, prx, sip, maxE };
	enum S{ out, in, maxS };

	NETDATA();
	SHP_Socket GS(int);//GetSocket
	EP GE(int);//GetEndpPoint
	IO& GI(int);//GetIO

private:
	vector<SHP_Socket> sockets;
	vector<EP> endPoints;
	vector<SHP_IO> ios;
};
typedef shared_ptr<NETDATA> SHP_NETDATA;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------

