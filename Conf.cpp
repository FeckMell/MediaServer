#pragma once
#include "stdafx.h"
#include "Conf.h"

void CConfPoint::ChangeMode(string SDP)
{
	std::size_t fd1, fd2;
	fd1 = SDP.find("inactive");
	fd2 = SDP_for_client.find("sendrecv");
	if ((fd1 != std::string::npos) && (fd2 != std::string::npos))
	{
		SDP_for_client.replace(fd2, 8, "inactive");
		SDP_for_client = ChangeVersion(SDP_for_client);
		mode = (!mode);
	}
	fd1 = SDP.find("sendrecv");
	fd2 = SDP_for_client.find("inactive");
	if ((fd1 != std::string::npos) && (fd2 != std::string::npos))
	{
		SDP_for_client.replace(fd2, 8, "sendrecv");
		SDP_for_client = ChangeVersion(SDP_for_client);
		mode = (!mode);
	}	
}
//--------------------------------------------------------------------------
string CConfPoint::ChangeVersion(string SDP)
{
	std::size_t fd1;
	fd1 = SDP.find("o=");
	if (fd1 != std::string::npos)
	{
		fd1 = SDP.find(" ", fd1 + 1);
		if (fd1 != std::string::npos)
		{
			fd1 = SDP.find(" ", fd1 + 1);
			if (fd1 != std::string::npos)
			{
				string temp = SDP.substr(fd1 + 1, SDP.find(" ", fd1 + 1));
				int temp2 = stoi(temp) + 1;
				SDP.replace(fd1, SDP.find(" ", fd1 + 1) - fd1 + 1, " " + to_string(temp2) + " ");
			}
		}
	}
	return SDP;
}
//--------------------------------------------------------------------------
void CConfPoint::ModifySDP(int a)
{
	std::size_t found = SDP_.find("m=audio");
	if (found != std::string::npos)
		SDP_ = SDP_.replace(found + 8, SDP_.find(" ", found + 10) - found - 8, to_string(my_port_ - 1000 - a));
	string rtp0 = "a=rtpmap:0";
	string rtp18 = "a=rtpmap:18";
	string rtp101 = "a=rtpmap:101";
	string rtpavp = "RTP/AVP";
	string fmtp = "a=fmtp:";
	// delete rtp 18, 101, 0
	found = SDP_.find(rtp18);
	if (found != std::string::npos)
		SDP_.replace(found, SDP_.find("\n", found + 1) - found + 1, "");

	found = SDP_.find(rtp101);
	if (found != std::string::npos)
		SDP_.replace(found, SDP_.find("\n", found + 1) - found + 1, "");

	found = SDP_.find(rtp0);
	if (found != std::string::npos)
		SDP_.replace(found, SDP_.find("\n", found + 1) - found + 1, "");

	found = SDP_.find(fmtp);
	while (found != std::string::npos)
	{
		SDP_.replace(found, SDP_.find("\n", found + 1) - found + 1, "");
		found = SDP_.find(fmtp);
	}
	// delete rtp 0

}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void CConfRoom::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	string time = "";
	steady_clock::time_point t1 = steady_clock::now();
	time += DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	CLogger.AddToLog(1, "\n" + time + "       " + a);
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
void CConfRoom::DeletePoint(string CallID)
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
	if (cllPoints_.size() > 1)
	{
		loggit("room size > 1");
		Start();
		loggit("CConfRoom::DeletePoint Start(); DONE");
		return;
	}
	if (cllPoints_.size() == 1)
	{
		loggit("room size == 1, Mixer->destroy_all();");
		//Mixer->destroy_all();
		boost::thread my_thread(&CRTPReceive::destroy_all, Mixer);
		my_thread.join();
		Mixer.reset();
		cout << "\nMixer pointer reseted";
		loggit("Mixer pointer reseted");
		return;
	}
	else
	{
		loggit("Should delete room");
		return;
	}
}
//--------------------------------------------------------------------------
void CConfRoom::NewInitPoint(string SDPff,string SDPfc, string CallID, int port)
{
	loggit("void CConfRoom::NewInitPoint(string SDP)");
	SHP_CConfPoint point(new CConfPoint());

	point->CallID_ = CallID;
	point->my_port_ = port;
	point->SDP_for_client = SDPfc;

	if (SDPff != "") 
	{
		point->mode = true;
		point->SDP_ = SDPff;
		point->ModifySDP(0);
		point->remote_port_ = stoi(MakeRemotePort(SDPff));
		point->remote_ip_ = MakeRemoteIP(SDPff);
	}
	cllPoints_.push_back(point);
	loggit("void CConfRoom::NewInitPoint(string SDP) ENDED");
}
//--------------------------------------------------------------------------
void CConfRoom::Start()
{
	NetworkData net = FillNetData();
	loggit("CConfRoom::Start() ");
	counter++;
	counter = counter % 2;
	if (on == false)
	{
		
		if (net.input_SDPs.size() >= 3)
		{
			on = true;
			loggit("mix->process_all() for " + std::to_string(net.input_SDPs.size()) + "clients");
			Mixer.reset(new CRTPReceive(net));
			Mixer->process_all(net);
		}
	}
	else
	{
		if (net.input_SDPs.size() >= 2)
		{
			loggit("mix->AddCall2 for " + std::to_string(net.input_SDPs.size()) + "clients");
			Mixer->add_track(net);
		}
	}
}
//--------------------------------------------------------------------------
NetworkData CConfRoom::FillNetData()
{
	NetworkData net;
	for (auto &entry : cllPoints_)
	{
		if (entry->mode == true)
		{
			//out << "net +1";
			entry->ModifySDP(/*1000 * counter*/0);
			net.input_SDPs.push_back(entry->SDP_);
			net.IPs.push_back(entry->remote_ip_);
			net.my_ports.push_back(entry->my_port_);
			net.remote_ports.push_back(entry->remote_port_);
		}
	}
	return net;
}
//--------------------------------------------------------------------------
SHP_CConfPoint CConfRoom::FindPoint(string CallID)
{
	for (auto &e : cllPoints_)
	{
		if (e->CallID_ == CallID)
			return e;
	}
	return nullptr;
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
string CConfRoom::ModifyPoint(SHP_CConfPoint Point, string SDPff)
{
	if (Point->SDP_ == "")
	{
		Point->SDP_ = SDPff;
		Point->ModifySDP(0);
		Point->remote_port_ = stoi(MakeRemotePort(SDPff));
		Point->remote_ip_ = MakeRemoteIP(SDPff);
		Point->mode = (!(Point->mode));
		Start();
		return "";
	}
	else
	{
		Point->ChangeMode(SDPff);
		Start();
		return Point->SDP_for_client;
	}
}
//--------------------------------------------------------------------------
