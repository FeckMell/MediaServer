#pragma once
#include "stdafx.h"
#include "Conf.h"
//#include <boost/thread/thread.hpp>
void StartRoom(SHP_CRTPReceive mixer)
{
	mixer->process_all();
}
void AddCall(SHP_CRTPReceive mixer, NetworkData net)
{
	mixer->add_track(net);
}
void CConfPoint::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	string time = "";
	steady_clock::time_point t1 = steady_clock::now();
	time += to_string(t->tm_year + 1900) + "." + to_string(t->tm_mon + 1) + "." + to_string(t->tm_mday) + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	
	fprintf(FileLogConfPoint, ("\n" + time + "       " + a/* + "\n//-------------------------------------------------------------------"*/).c_str());
	fflush(FileLogConfPoint);
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
	for (auto & entry : cllPoints_)
	{
		if (CallID == entry->GetID())
		{
			cllPoints_.erase(std::remove(cllPoints_.begin(), cllPoints_.end(), entry), cllPoints_.end());
			break;
		}
	}
	if (cllPoints_.size() >= 2)
	{
		Start();
		return 1;
	}
	else
	{
		cout << "Mixer->destroy_all();";
		Mixer->destroy_all();
		Mixer.reset();
		return -1;
	}
		
}
//--------------------------------------------------------------------------
void CConfRoom::NewInitPoint(string SDP, string CallID, int port)
{
	loggit("void CConfRoom::NewInitPoint(string SDP)");
	SHP_CConfPoint point(new CConfPoint());

	point->SetID(CallID);
	point->SetMyPort(port);
	point->SetSDP(SDP);
	point->ModifySDP(0);
	
	point->SetRemotePort(atoi(MakeRemotePort(SDP).c_str()));
	point->SetRemoteIP(MakeRemoteIP(SDP));

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
	counter = counter % 3;
	string logSDP = "\n";
	if (on == false)
	{
		for (auto &entry : cllPoints_)
		{
			if (entry->mode == true)
			{
				entry->ModifySDP(1000*counter);
				logSDP += entry->GetSDP() + "\n";

				net.input_SDPs.push_back(entry->GetSDP());
				net.IPs.push_back(entry->GetRemoteIP());
				net.my_ports.push_back(entry->GetMyPort());
				net.remote_ports.push_back(entry->GetRemotePort());
			}
		}

		on = true;
		loggit("Creating Mixer for SDPs:" + logSDP);
		
		Mixer.reset(new CRTPReceive(net));
		boost::thread my_thread(&StartRoom, Mixer);
		my_thread.detach();
		loggit("mix->process_all();");
	}
	else
	{
		for (auto &entry : cllPoints_)
		{
			if (entry->mode == true)
			{
				entry->ModifySDP(1000*counter);
				logSDP += entry->GetSDP() + "\n";

				net.input_SDPs.push_back(entry->GetSDP());
				net.IPs.push_back(entry->GetRemoteIP());
				net.my_ports.push_back(entry->GetMyPort());
				net.remote_ports.push_back(entry->GetRemotePort());
			}

		}
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
		if (e->GetID() == CallID)
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
		SDP_ = SDP_.replace(found + 8, SDP_.find(" ", found + 10) - found - 8, to_string(GetMyPort() - 1000 - a));
}