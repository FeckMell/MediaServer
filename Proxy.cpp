#pragma once
#include "stdafx.h"
#include "Proxy.h"

void Proxy::loggit(string a)
{
	using namespace std;
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	string time = "";
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	time += DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	CLogger.AddToLog(7, "\n" + time + "       " + a + "\n//-------------------------------------------------------------------");
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
Proxy::Proxy(std::string SDP, int my_port, string ID, string CallID)
{
	loggit("Created Proxy with ID=" + ID + " for client=" + CallID + " with SDP:" + SDP + " \nmy_port=" + std::to_string(my_port));
	ID_ = ID;
	CallID_.push_back(CallID);
	std::string remote_ip = MakeRemoteIP(SDP);
	int remote_port = stoi(MakeRemotePort(SDP));

	net_.input_SDPs.push_back(SDP);
	net_.IPs.push_back(remote_ip);
	net_.remote_ports.push_back(remote_port);
	net_.my_ports.push_back(my_port);

	SHP_Socket a;
	a.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), my_port)));
	vecSock.push_back(a);
	vecEndpoint.push_back(udp::endpoint(boost::asio::ip::address::from_string(remote_ip), remote_port));

	++size;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void Proxy::NewPoint(std::string SDP, int my_port, string CallID)
{
	loggit("New Point for Proxy with ID=" + ID_ + " for client=" + CallID + " with SDP:" + SDP + " \nmy_port=" + std::to_string(my_port));
	CallID_.push_back(CallID);

	std::string remote_ip = MakeRemoteIP(SDP);
	int remote_port = stoi(MakeRemotePort(SDP));

	net_.input_SDPs.push_back(SDP);
	net_.IPs.push_back(remote_ip);
	net_.remote_ports.push_back(remote_port);
	net_.my_ports.push_back(my_port);

	SHP_Socket a;
	a.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), my_port)));
	vecSock.push_back(a);
	vecEndpoint.push_back(udp::endpoint(boost::asio::ip::address::from_string(remote_ip), remote_port));

	++size;
	if (size > 2) return;
	Start();
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void Proxy::Start()
{
	loggit("Send started");
	boost::shared_ptr<boost::thread> thread1(new boost::thread(&Proxy::process, this, 0));
	receive_threads.push_back(thread1);
	boost::shared_ptr<boost::thread> thread2(new boost::thread(&Proxy::process, this, 1));
	receive_threads.push_back(thread2);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void Proxy::process(int i)
{
	while (on == true)
	{
		try
		{
			int szPack = 0;
			char data_[1000];
			szPack = vecSock[i]->receive_from(boost::asio::buffer(data_, 1000), vecEndpoint[i]);
			if (szPack > 12)
				vecSock[i]->send_to(boost::asio::buffer(data_, szPack), vecEndpoint[(i + 1) % 2]);
			else
			{
				cout << "\nreceive???";
				system("pause");
			}
		}
		catch (std::exception& e)
		{
			cout << "\nreceive exeption (ok)" + std::to_string(*e.what());
		}
	}
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int Proxy::DeletePoint(string CallID)
{
	int num = -1;
	for (int i = 0; i < 2; ++i)
		if (CallID_[i] == CallID) num = i;
	if (num != -1)
	{
		on = false;
		receive_threads[num]->~thread();
		vecSock[num]->close();
	}
	--size;
	return net_.my_ports[num];
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------