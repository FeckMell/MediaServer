#pragma once
#include "../../stdafx.h"
#include "Socket.h"
#include "Communication.h"
#include "../Protocols/SDP.h"
#include "../Protocols/IPL.h"
#include "../Basic/Basic.h"
#include "../Basic/Timer.h"
#include "Logs.h"


class Caller
{
public:

	/*Main public activity*/
	Caller(string call_id_);
	~Caller();
	void StopActivity();

	bool Check();

	void SetSDP(string type_, SHP_SDP new_sdp_);
	SHP_SDP GetSDP(string type_);

	void AddReceiveFunc(string func_name_, boost::function<void(SHP_SockBuf)> func_);
	void RemReceiveFunc(string func_name_);

	void SendTo(SHP_SockBuf mes_);
	/*\Main public activity*/

	void SetParam(string name_, string value_);
	string Param(string name_);

private:

	/*Main private activity and data*/
	void ReceiveFromSocket(boost::system::error_code ec_, size_t size_);

	SHP_Socket shpSocket;
	RTP headerRTP;

	SHP_SDP clientSDP;
	SHP_SDP serverSDP;

	SHP_Timer shpTimer;

	map<string, string> mapData; /*CallID,*/
	map<string, boost::function<void(SHP_SockBuf)>> mapFunc;
	mutex funcMutex;
	/*\Main private activity and data*/

};
typedef shared_ptr<Caller> SHP_Caller;

class CallerStore
{
public:

	/*Main public activity*/
	static void Init();

	static void CreateCaller(string call_id_);
	static SHP_Caller GetCaller(string call_id_);
	static void DelCaller(string call_id_);
	/*\Main public activity*/

	static void CheckOverload(Caller* target_);

private:

	/*Main private activity and data*/
	static void FakeReceive(boost::system::error_code ec_, size_t size_);

	static SHP_IO ioForAll; /*Caller Receive Thread*/
	static SHP_Socket fakeSocket;
	static SHP_thread receiveThread;

	static map<string, SHP_Caller> mapCaller;
	static mutex thisMutex;
	/*\Main private activity and data*/

};

