#pragma once
#include "../../stdafx.h"
#include "../Basic/Basic.h"
#include "../Protocols/RTP.h"
#include "Config.h"


class Socket
{
public:
	/*Main public activity*/
	Socket(string s_port_, SHP_IO io_);
	Socket(string s_ip_, string s_port_, SHP_IO io_);
	~Socket();

	void SetEndPoint(string c_ip_, string c_port_);
	EP GetEndPoint();

	uint8_t* Buffer();

	void Cancel();

	void SendTo(SHP_SockBuf mess_);
	void SendTo(string mess_, EP where_);

	void AsyncReceiveFrom(boost::function<void(boost::system::error_code ec_, size_t size_)> bind_func_);

	/*\Main public activity*/

	string Param(string name_);
	private:

	/*Main private activity and data*/
	BSocket socket;
	EP endPoint;
	SHP_IO io;

	uint8_t buffer[2048];
	//RTP rtp; ->Caller
	/*\Main private activity and data*/
private:

};
typedef shared_ptr<Socket> SHP_Socket;

class SocketStore
{
public:

	/*Main public activity*/
	static string CreateSocket(SHP_IO io_);
	static SHP_Socket GetSocket(string s_port_);
	static void FreeSocket(string s_port_);
	/*\Main public activity*/

private:

	/*Main private activity and data*/
	static map<string, SHP_Socket> mapSocket;
	/*\Main private activity and data*/

	static string ReservePort();
	static void FreePort(string port_);

	static vector<int> vecPort;

};

