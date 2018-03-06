#pragma once
#include "stdafx.h"
#include "SL_Config.h"
#include "SL_BasicStructs.h"
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class SOCK
{
public:
	SOCK(string, int, SHP_IO);
	~SOCK();

	void ChangeIO(SHP_IO);
	void AsyncReceive(boost::function<void(boost::system::error_code, size_t)> boostbind_);
	void SendTo(uint8_t*, int);
	void SetEndPoint(string, string);
	uint8_t* GetRTP();
	uint8_t* GetRTP(int);

	EP endPoint;
	uint8_t buffer[2048];
	SHP_IO io;
	boost::asio::ip::udp::socket s;

private:

	RTP_struct rtp;

};
typedef shared_ptr<SOCK> SHP_SOCK;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class SSTORAGE
{
public:

	static SHP_SOCK GetSocket(string);

	static string ReservePort();
	static void FreePort(string);

	static string ReserveEventID();
	static void FreeEventID(string);

	static map<string, SHP_SOCK> mapSocks;

private:

	static vector<int> usedPorts;
	static vector<int> usedEventIDs;

};