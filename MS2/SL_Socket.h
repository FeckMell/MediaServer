#pragma once
#include "stdafx.h"
#include "SL_RTP.h"

class SOCKET
{
public:

	SOCKET(string ip_, string port_);
	~SOCKET();

	void AsyncReceive(boost::function<void(boost::system::error_code, size_t)> function_);
	void AsyncSendTo(boost::function<void(boost::system::error_code, size_t)> function_);
	void SendTo(uint8_t* data_, int size_);
	
	void IOAction(string action_);
	void SocketAction(string action_);

	void ChangeIO(SHP_IO new_io_);
	void SetEndPoint(string ip_, string port_);

	string Param(string what_); /*string(_buffer) , params, string((uint8_t*)_rtpheader)*/
	SHP_IO GetIO();
	string PrintAll();

private:

	SHP_IO _io;
	boost::asio::ip::udp::socket _socket;
	EP _endpoint;
	uint8_t _buffer[2048];
	RTP _rtpheader;

};
typedef shared_ptr<SOCKET> SHP_SOCKET;

class SOCKETSTORE
{
public:

	static SHP_SOCKET GetSocket(string port_); //if port_="" reserve port
	static void DeleteSocket(string port_); //on point destructor
	static string ReservePort();

	static string PrintAll();

private:

	static void FreePort(string port_);

	static vector<int> _vecports;
	static map<string, SHP_SOCKET> _mapsockets;

};