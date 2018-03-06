#pragma once
#include "../../stdafx.h"
#include "Simple.h"

class Socket
{
public:
	Socket(string c_ip_, string c_port_, string s_ip_, string s_port_, SHP_IO io_);
	~Socket();
	void Stop();

	EP GetEndPoint();
	void ResetEndPoint();
	void SendTo(SHP_SockBuf buf_);
	void AsyncReceiveFrom(boost::function<void(boost::system::error_code ec_, size_t size_)> bind_func_);
	uint8_t* Data();

private:

	EP endPoint;
	uint8_t buffer[2048];
	SHP_IO io;
	BSocket bSocket;
	

}; typedef shared_ptr<Socket> SHP_Socket;