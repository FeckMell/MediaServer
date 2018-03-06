#include "stdafx.h"
#include "Proxy.h"



void Proxy::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	steady_clock::time_point t1 = steady_clock::now();
	string result = DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	result += " ID=" + ID_ + " thread=" + boost::to_string(this_thread::get_id()) + "      ";
	CLogger->AddToLog(7, "\n" + result + a);
}
void ProxyCaller::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	steady_clock::time_point t1 = steady_clock::now();
	string result = DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	result += " ID=" + CallID_ + " thread=" + boost::to_string(this_thread::get_id()) + "      ";
	CLogger->AddToLog(7, "\n" + result + a);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
Proxy::Proxy(std::string SDP, int my_port, string ID, string CallID) : ID_(ID)
{
	loggit("Created Proxy with ID=" + ID + " for client=" + CallID + " with SDP:" + SDP + " \nmy_port=" + std::to_string(my_port));
	SHP_ProxyCaller caller(new ProxyCaller(SDP, my_port, CallID, io_service_));
	callers_.push_back(caller);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void Proxy::NewPoint(string SDP, int my_port, string CallID)
{
	loggit("Adding point for Proxy with ID=" + ID_ + " for client=" + CallID + " with SDP:" + SDP + " \nmy_port=" + std::to_string(my_port));
	SHP_ProxyCaller caller(new ProxyCaller(SDP, my_port, CallID, io_service_));
	callers_.push_back(caller);
	Start();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Proxy::ModifyPoint(string SDP, string CallID)
{
	loggit("Modify point " + CallID + " with SDP=\n" + SDP);
	for (int i = 0; i < (int)callers_.size(); ++i)
	{
		if (callers_[i]->CallID_ == CallID)
		{
			loggit("Point found!");
			callers_[i]->Endpoint_rec = udp::endpoint(boost::asio::ip::address::from_string(MakeRemoteIP(SDP)), stoi(MakeRemotePort(SDP)));
			callers_[i]->SetState(ProxyCaller::RECORD);
			return;
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int Proxy::DeletePoint(string CallID)
{
	loggit("Delete point with Call ID = " + CallID);
	int del = -1;
	int port;
	for (int i = 0; i < (int)callers_.size(); ++i)
	{
		callers_[i]->SetState(ProxyCaller::OFF);
		if (callers_[i]->CallID_ == CallID)
			del = i;
		loggit("del=" + std::to_string(del));
	}
	if (del != -1)
	{
		port = callers_[del]->my_port_;
		loggit("port to delete=" + std::to_string(port));
		callers_[del]->Delete();
		loggit("Delete called");
		receive_threads[0]->join();
		callers_.erase(std::remove(callers_.begin(), callers_.end(), callers_[del]), callers_.end());
	}
	loggit("delete done");
	return port;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Proxy::Run_io()
{
	io_service_.run();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Proxy::Start()
{
	loggit("Starting");
	for (int i = 0; i < (int)callers_.size(); ++i)
	{
		callers_[i]->SetState(ProxyCaller::ON);
		callers_[i]->Sock->async_receive_from(boost::asio::buffer(callers_[i]->RawBuf.data, 8000), callers_[i]->Endpoint,
			boost::bind(&ProxyCaller::Receive, callers_[i], _1, _2, callers_[1-i]));
	}
	boost::shared_ptr<boost::thread> thread_io(new boost::thread(&Proxy::Run_io,this));
	receive_threads.push_back(thread_io);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
ProxyCaller::ProxyCaller(string SDP, int my_port, string CallID, asio::io_service &service) :
CallID_(CallID), io_service_(service), SDP_(SDP), my_port_(my_port)
{
	//remote_port_ = stoi(MakeRemotePort(SDP_));
	//remote_IP_ = MakeRemoteIP(SDP_);
	loggit("create proxycaller with:\nCallID=" + CallID_ + " port=" + to_string(my_port_) + " and SDP=\n" + SDP_);
	Sock.reset(new boost::asio::ip::udp::socket(io_service_));
	Sock->open(udp::v4());
	Sock->set_option(boost::asio::ip::udp::socket::reuse_address(true));
	Sock->bind(udp::endpoint(udp::v4(), my_port_));

	Endpoint = udp::endpoint(boost::asio::ip::address::from_string(MakeRemoteIP(SDP_)), stoi(MakeRemotePort(SDP_)));
	loggit("DONE");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void ProxyCaller::Receive(boost::system::error_code ec, size_t szPack, SHP_ProxyCaller another)
{
	loggit("Receive");
	if (state_ == ON)
	{
		loggit("Receive \"on\"");
		another->Sock->send_to(boost::asio::buffer(RawBuf.data, szPack), another->Endpoint);
		Sock->async_receive_from(boost::asio::buffer(RawBuf.data, 8000), Endpoint,
			boost::bind(&ProxyCaller::Receive, this, _1, _2, another));
	}
	if (state_ == RECORD)
	{
		loggit("Receive \"RECORD\"");
		another->Sock->send_to(boost::asio::buffer(RawBuf.data, szPack), another->Endpoint);
		Sock->send_to(boost::asio::buffer(RawBuf.data, szPack), Endpoint_rec);
		Sock->async_receive_from(boost::asio::buffer(RawBuf.data, 8000), Endpoint,
			boost::bind(&ProxyCaller::Receive, this, _1, _2, another));
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void ProxyCaller::Delete()
{
	loggit("DELETE");
	Sock->cancel();
	io_service_.reset();
	Sock->close();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------