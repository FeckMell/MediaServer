#pragma once
#ifdef WIN32
#include "stdafx.h"
#endif
#ifdef linux
#include "stdinclude.h"
#endif
#include "Logger.h"
#include "Structs.h"
#include "Functions.h"

extern Logger* CLogger;
using namespace std;
using namespace std::chrono;
class Proxy
{
public:
	Proxy(std::string SDP, int my_port, string ID, string CallID);
	string GetID(){ return ID_; }
	void NewPoint(std::string SDP, int my_port, string CallID);
	int DeletePoint(string CallID);
	int Size(){ return size; }
private:
	void Proxy::loggit(string a);
	void Start();
	void process(int i);
	boost::asio::io_service io_service_;
	NetworkData net_;
	std::vector<udp::endpoint> vecEndpoint;
	std::vector<boost::shared_ptr<boost::thread>> receive_threads;
	std::vector<SHP_Socket> vecSock;
	std::vector<string> CallID_;
	string ID_;
	bool on = false;
	int size = 0;
};
typedef std::shared_ptr<Proxy> SHP_Proxy;