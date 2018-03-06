// Redirector.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
using boost::asio::ip::udp;
using namespace std;

const int RedirectSwitchPort = 2427;
const int RedirectServerPort = 2428;

string RedirectSwitchIP;// = "10.77.7.5";
string RedirectServerIP;// = "10.77.7.6";

uint8_t* RedirectSwitchData = new uint8_t[8000];
uint8_t* RedirectServerData = new uint8_t[8000];

udp::endpoint SwitchLocation;
udp::endpoint ServerLocation;

std::shared_ptr<udp::socket> RedirectSwitchSocket;
std::shared_ptr<udp::socket> RedirectServerSocket;

boost::asio::io_service service;



void ReceiveSwitch(boost::system::error_code ec, size_t szPack);
void ReceiveServer(boost::system::error_code ec, size_t szPack);

int main()
{
	cout << "DM: 10.77.7.13\nServerWin: 10.77.1.61\nLinuxDebServer:10.77.7.6";
	cout << "\nEnter this machine IP:\n";
	RedirectSwitchIP = "10.77.7.13";//
	cout << "10.77.7.13";//
	//std::cin >> RedirectSwitchIP;
	cout << "\nEnter server IP:\n";
	RedirectServerIP = "10.77.1.61";//
	cout << "10.77.1.61\n";//
	//std::cin >> RedirectServerIP;
	ServerLocation = udp::endpoint(boost::asio::ip::address::from_string(RedirectServerIP), 2427);

	RedirectSwitchSocket.reset(new boost::asio::ip::udp::socket(service));
	RedirectSwitchSocket->open(udp::v4());
	RedirectSwitchSocket->set_option(boost::asio::ip::udp::socket::reuse_address(true));
	RedirectSwitchSocket->bind(udp::endpoint(udp::v4(), RedirectSwitchPort));

	RedirectServerSocket.reset(new boost::asio::ip::udp::socket(service));
	RedirectServerSocket->open(udp::v4());
	RedirectServerSocket->set_option(boost::asio::ip::udp::socket::reuse_address(true));
	RedirectServerSocket->bind(udp::endpoint(udp::v4(), RedirectServerPort));
	cout << "init done.";
	RedirectSwitchSocket->async_receive_from(boost::asio::buffer(RedirectSwitchData, 8000), SwitchLocation,
		boost::bind(ReceiveSwitch, _1, _2));
	RedirectServerSocket->async_receive_from(boost::asio::buffer(RedirectServerData, 8000), ServerLocation,
		boost::bind(ReceiveServer, _1, _2));
	cout << "io_service run.";
	//
	for (;;)
	{
		RedirectSwitchSocket->send_to(boost::asio::buffer("123123", 6), udp::endpoint(boost::asio::ip::address::from_string(RedirectServerIP), 55595));
		this_thread::sleep_for(chrono::milliseconds(20));
	}
	cout << "\nsent";
	//
	service.run();

	return 0;
}

void ReceiveSwitch(boost::system::error_code ec, size_t szPack)
{
	if (!ec)
	{
		cout << "\nReceive from Switch";
		RedirectServerSocket->send_to(boost::asio::buffer(RedirectSwitchData, szPack), ServerLocation);
		RedirectSwitchSocket->async_receive_from(boost::asio::buffer(RedirectSwitchData, 8000), SwitchLocation,
			boost::bind(ReceiveSwitch, _1, _2));
	}
}

void ReceiveServer(boost::system::error_code ec, size_t szPack)
{
	if (!ec)
	{
		cout << "\nReceive from Server";
		RedirectSwitchSocket->send_to(boost::asio::buffer(RedirectServerData, szPack), SwitchLocation);
		RedirectServerSocket->async_receive_from(boost::asio::buffer(RedirectServerData, 8000), ServerLocation,
			boost::bind(ReceiveServer, _1, _2));
	}
}