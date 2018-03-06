#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "Structs.h"
#include "Functions.h"

extern Logger* CLogger;
using namespace std;
using namespace std::chrono;

class ProxyCaller;
typedef std::shared_ptr<ProxyCaller> SHP_ProxyCaller;

class Proxy
{
public:
	enum State {OFF, ON, PAUSED, DESTROY};
	Proxy(string SDP, int my_port, string ID, string CallID);
	string GetID(){ return ID_; }
	void Run_io();

	void NewPoint(string SDP, int my_port, string CallID);
	void ModifyPoint(string SDP, string CallID);
	int DeletePoint(string CallID);

	int Size(){ return callers_.size(); }
private:
	void loggit(string a);
	void Start();
	vector<boost::shared_ptr<boost::thread>> receive_threads;
	vector<SHP_ProxyCaller> callers_;
	boost::asio::io_service io_service_;
	string ID_;
	State state_ = OFF;
};
typedef std::shared_ptr<Proxy> SHP_Proxy;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class ProxyCaller
{
public:
	enum State { OFF, ON, RECORD };
	ProxyCaller(string SDP, int my_port, string CallID, asio::io_service &service);
	void SetState(State st){ state_ = st; }
	void Delete();

	void Receive(boost::system::error_code ec, size_t szPack, SHP_ProxyCaller another);
	int my_port_;
	//int remote_port_;
	//string remote_IP_;
	SHP_Socket Sock;
	udp::endpoint Endpoint;
	udp::endpoint Endpoint_rec;
	string SDP_;
	Data RawBuf;
	State state_ = OFF;
	string CallID_;
private:
	void loggit(string a);
	boost::asio::io_service &io_service_;


};
typedef std::shared_ptr<ProxyCaller> SHP_ProxyCaller;

