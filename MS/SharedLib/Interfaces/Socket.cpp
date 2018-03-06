#include "stdafx.h"
#include "Socket.h"

map<string, SHP_Socket> SocketStore::mapSocket = {};
vector<int> SocketStore::vecPort = {};

Socket::Socket(string s_port_, SHP_IO io_) :socket(*io_.get()), io(io_)
{
	EP ep(boost::asio::ip::address::from_string(CFG::Param("OuterIP")), stoi(s_port_));
	socket.open(boost::asio::ip::udp::v4());
	socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	socket.bind(ep);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Socket::Socket(string s_ip_, string s_port_, SHP_IO io_) :socket(*io_.get()), io(io_)
{
	EP ep(boost::asio::ip::address::from_string(s_ip_), stoi(s_port_));
	socket.open(boost::asio::ip::udp::v4());
	socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	socket.bind(ep);
}
Socket::~Socket()
{

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Socket::SetEndPoint(string c_ip_, string c_port_)
{
	if (c_ip_ != "")
	{
		endPoint = EP(boost::asio::ip::address::from_string(c_ip_), stoi(c_port_));
	}
	else
	{
		endPoint = EP();
	}
}
EP Socket::GetEndPoint()
{
	return endPoint;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Socket::Param(string name_)
{
	if (name_ == "ClientIP") return endPoint.address().to_string();
	if (name_ == "ClientPort") return to_string(endPoint.port());
	if (name_ == "ServerIP") return socket.local_endpoint().address().to_string();
	if (name_ == "ServerPort") return to_string(socket.local_endpoint().port());
	else return "";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Socket::SendTo(SHP_SockBuf mess_)
{
	socket.send_to(
		boost::asio::buffer(mess_->Data(), mess_->Size()),
		endPoint
		);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Socket::SendTo(string mess_, EP where_)
{
	socket.send_to(
		boost::asio::buffer(mess_),
		where_
		);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
uint8_t* Socket::Buffer()
{
	return buffer;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Socket::AsyncReceiveFrom(boost::function<void(boost::system::error_code ec_, size_t size_)> bind_func_)
{
	socket.async_receive_from(
		boost::asio::buffer(buffer),
		endPoint,
		bind_func_
		);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Socket::Cancel()
{
	socket.cancel();
	socket.close();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string SocketStore::CreateSocket(SHP_IO io_)
{
	string new_server_port = ReservePort();
	SHP_Socket new_socket; new_socket.reset(new Socket(new_server_port, io_));
	mapSocket[new_server_port] = new_socket;
	return new_server_port;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Socket SocketStore::GetSocket(string s_port_)
{
	return mapSocket[s_port_];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SocketStore::FreeSocket(string s_port_)
{
	FreePort(s_port_);
	mapSocket.erase(s_port_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string SocketStore::ReservePort()
{
	int free_port = stoi(CFG::Param("RTPPort"));
	if (vecPort.size() == 0)
	{
		vecPort.push_back(free_port);
		return to_string(free_port);
	}
	for (unsigned i = 0; i < vecPort.size(); ++i)
	{
		if (vecPort[i] != free_port)
		{
			vecPort.push_back(free_port);
			sort(vecPort.begin(), vecPort.end());
			return to_string(free_port);
		}
		free_port += 2;
	}
	vecPort.push_back(free_port);
	sort(vecPort.begin(), vecPort.end());
	return to_string(free_port);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SocketStore::FreePort(string port_)
{
	vecPort.erase(remove(vecPort.begin(), vecPort.end(), stoi(port_)), vecPort.end());
}