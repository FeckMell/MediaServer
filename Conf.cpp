#pragma once
#include "stdafx.h"
#include "Conf.h"
//#include <boost/thread/thread.hpp>
void StartRoom(SHP_CRTPReceive mixer, NetworkData net)
{
	LogMain("StartRoom");
	mixer->process_all(net);
	LogMain("StartRoom DONE");
}
void AddCall(SHP_CRTPReceive mixer, NetworkData net)
{
	LogMain("AddCall");
	mixer->add_track(net);
	LogMain("AddCall DONE");
}

void CConfRoom::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	string time = "";
	steady_clock::time_point t1 = steady_clock::now();
	time += to_string(t->tm_year + 1900) + "." + to_string(t->tm_mon + 1) + "." + to_string(t->tm_mday) + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);

	fprintf(FileLogConfRoom, ("\n" + a + "\n//-------------------------------------------------------------------").c_str());
	fflush(FileLogConfRoom);
}
//--------------------------------------------------------------------------
string CConfRoom::Make_addr_from_SDP(string output_SDP)
{
	string result = "rtp://";
	string temp;
	std::size_t found = output_SDP.find("c=IN IP4");
	if (found != std::string::npos)
		temp = output_SDP.substr(found + 9, output_SDP.find("\n", found+1)-found - 9);
	result += temp + ":";
	found = output_SDP.find("m=audio");
	if (found != std::string::npos)
		temp = output_SDP.substr(found + 8, output_SDP.find(" ", found + 10)- found - 8);
	result += temp;
	temp.clear();
	return result;
}
//--------------------------------------------------------------------------
int CConfRoom::DeletePoint(string CallID)
{
	loggit("delete point for ID=" + CallID);
	for (auto & entry : cllPoints_)
	{
		loggit("Compare Point ID=" + entry->CallID_);
		if (CallID == entry->CallID_)
		{
			loggit("Found Point");
			cllPoints_.erase(std::remove(cllPoints_.begin(), cllPoints_.end(), entry), cllPoints_.end());
			loggit("Point erased");
			break;
		}
	}
	loggit("???");
	if (cllPoints_.size() >= 2)
	{
		loggit("room size>=2");
		Start();
		loggit("CConfRoom::DeletePoint Start(); DONE");
		return 1;
	}
	else
	{
		loggit("room size<2, Mixer->destroy_all();");
		cout << "Mixer->destroy_all();";
		Mixer->destroy_all();
		loggit("reseting Mixer pointer");
		Mixer.reset();
		loggit("Mixer pointer reseted");
		return -1;
	}		
}
//--------------------------------------------------------------------------
void CConfRoom::NewInitPoint(string SDP, string CallID, int port)
{
	loggit("void CConfRoom::NewInitPoint(string SDP)");
	SHP_CConfPoint point(new CConfPoint());

	point->CallID_=CallID;
	point->my_port_=port;
	point->SDP_=SDP;
	point->ModifySDP(0);
	
	point->remote_port_=atoi(MakeRemotePort(SDP).c_str());
	point->remote_ip_=MakeRemoteIP(SDP);

	cllPoints_.push_back(point);
	if (cllPoints_.size() >= 3)
	{
		Start();
	}

	loggit("void CConfRoom::NewInitPoint(string SDP) ENDED");
}
//--------------------------------------------------------------------------
void CConfRoom::Start()
{
	loggit("CConfRoom::Start()");
	NetworkData net;
	counter++;
	counter = counter % 2;
	string logSDP = "\n";
	if (on == false)
	{
		for (auto &entry : cllPoints_)
		{
			if (entry->mode == true)
			{
				entry->ModifySDP(1000*counter);
				logSDP += entry->SDP_ + "\n";

				net.input_SDPs.push_back(entry->SDP_);
				net.IPs.push_back(entry->remote_ip_);
				net.my_ports.push_back(entry->my_port_);
				net.remote_ports.push_back(entry->remote_port_);
			}
		}

		on = true;
		loggit("Creating Mixer for SDPs:" + logSDP);
		
		Mixer.reset(new CRTPReceive(net));
		boost::thread my_thread(&StartRoom, Mixer, net);
		my_thread.detach();
		loggit("mix->process_all();");
	}
	else
	{
		loggit("In Start(); on == true");
		for (auto &entry : cllPoints_)
		{
			if (entry->mode == true)
			{
				entry->ModifySDP(1000*counter);
				logSDP += entry->SDP_ + "\n";

				net.input_SDPs.push_back(entry->SDP_);
				net.IPs.push_back(entry->remote_ip_);
				net.my_ports.push_back(entry->my_port_);
				net.remote_ports.push_back(entry->remote_port_);
			}

		}
		loggit("Size="+to_string(net.input_SDPs.size())+"Creating Mixer for SDPs:" + logSDP);
		boost::thread my_thread(&AddCall, Mixer, net);
		my_thread.detach();
		loggit("mix->AddCall;");
	}
	net.free();
	logSDP.clear();
}
//--------------------------------------------------------------------------
SHP_CConfPoint CConfRoom::FindPoint(string CallID)
{
	for (auto &e : cllPoints_)
	{
		if (e->CallID_ == CallID)
			return e;
	}
	return NULL;
}
//--------------------------------------------------------------------------
string CConfRoom::MakeRemoteIP(string SDP)
{
	std::size_t found = SDP.find("c=IN IP4");
	if (found != std::string::npos)
		return SDP.substr(found + 9, SDP.find("\n", found + 1) - found - 9);
	return "";
}
//--------------------------------------------------------------------------
string CConfRoom::MakeRemotePort(string SDP)
{
	std::size_t found = SDP.find("m=audio");
	if (found != std::string::npos)
		return SDP.substr(found + 8, SDP.find(" ", found + 10) - found - 8);
	return "";
}
//--------------------------------------------------------------------------
void CConfPoint::ModifySDP(int a)
{
	std::size_t found = SDP_.find("m=audio");
	if (found != std::string::npos)
		SDP_ = SDP_.replace(found + 8, SDP_.find(" ", found + 10) - found - 8, to_string(my_port_ - 1000 - a));
}