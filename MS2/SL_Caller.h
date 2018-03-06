#include "stdafx.h"
#include "SL_Socket.h"
#include "SL_SDP.h"
#include "SL_JitterBuffer.h"
#include "SL_Time.h"

class CALLER
{
public:

	CALLER(SHP_SDP s_SDP_);
	~CALLER();

	void ModifyClientSDP(SHP_SDP c_SDP_);

	SHP_SDP GetSDP(string name_);
	SHP_SOCKET GetSocket();
	JITTER* GetJitter();
	TIME* GetTimer();

	string Param(string name_);
	string PrintAll();

private:

	TIME _timer;

	SHP_SDP _clientSDP;
	SHP_SDP _serverSDP;

	SHP_SOCKET _socket;
	JITTER _jitter;
	map<string, string> _data;

};
typedef shared_ptr<CALLER> SHP_CALLER;

class CALLERSTORE
{
public:

	static SHP_CALLER CreateCaller(SHP_SDP s_SDP_);
	static SHP_CALLER GetCaller(string s_port_);
	static void DeleteCaller(string s_port_);

	static void CheckCallers(string c_ip_, string c_port_);
	static bool CheckExistance(string callid_, string c_ip_, string c_port_);

private:

	static map<string, SHP_CALLER> _mapcallers;

};