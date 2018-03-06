#pragma once
#include "stdafx.h"
#include "ConfControl.h"
void SendAnn(SHP_Ann ann, string file)
{
	//out << "\nthread SendAnn " << std::this_thread::get_id();
	ann->Send(file);
}

void ConfControl::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	string time = "";
	steady_clock::time_point t1 = steady_clock::now();
	time += DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	CLogger.AddToLog(6, "\n" + time + "     " + a);
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
int ConfControl::SetRoomID()
{
	if (RoomsID_.size() == 0) { RoomsID_.push_back(1); return 1; }
	for (unsigned i = 1; i < RoomsID_.size(); ++i)
	{
		if (RoomsID_[i-1] != i){ RoomsID_.push_back(i); return i; }
	}
	RoomsID_.push_back(RoomsID_.size());
	std::sort(RoomsID_.begin(), RoomsID_.end());
	return RoomsID_.size();
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
std::string ConfControl::GenSDP(int Port, MGCP &mgcp)
{
	steady_clock::time_point t1 = steady_clock::now();
	string date = DateStr;
	size_t fd;
	while ((fd = date.find("-")) != string::npos)
	{
		date.replace(fd,1,"");
	}
	auto f = boost::format(string("\n\nv=0\no=- %3% 0 IN IP4 %1%\ns=%4%\nc=IN IP4 %1%\nt=0 0\na=tool:libavformat 57.3.100\nm=audio %2% RTP/AVP 8\na=rtpmap:8 PCMA/8000\na=ptime:20\na=sendrecv\n")); // формируем тип ответа
	return str(f %my_IP %Port % (date + boost::to_string(t1.time_since_epoch().count() % 10000000)) % mgcp.paramC);
	
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void ConfControl::proceedCRCX(MGCP &mgcp, const udp::endpoint& udpTO)
{
	loggit(mgcp.EventEx + " CRCX for " + mgcp.paramC);

	switch (mgcp.Event)
	{
	case MGCP::ann:
	{
		auto Port = GetFreePort();
		mgcp.EventNum = boost::to_string(SetRoomID());
		loggit("creating Ann with ID=" + mgcp.EventNum + " my_port=" + boost::to_string(Port));
		SHP_Ann Ann(new Ann(mgcp.SDP, Port, mgcp.paramC));
		AnnVec_.push_back(Ann);

		server->reply(mgcp.ResponseOK(200, mgcp.EventS) + GenSDP(Port, mgcp), udpTO);
		loggit("Ann created");
		return;
	}//case ann

	case MGCP::cnf:
	{
		if (mgcp.EventS + mgcp.EventNum == "cnf/$")
		{
			auto Room = CreateNewRoom();
			auto Port = GetFreePort();
			loggit("creating Conf with ID=" + boost::to_string(Room->GetRoomID()) + " my_port=" + boost::to_string(Port));
			auto my_SDP = GenSDP(Port, mgcp);
			Room->NewInitPoint(mgcp.SDP, my_SDP, mgcp.paramC, Port);
			mgcp.EventNum = boost::to_string(Room->GetRoomID());
			server->reply(mgcp.ResponseOK(200, mgcp.EventS) + my_SDP, udpTO);
			loggit("Conf created");
			return;
		}
		else
		{
			auto Room = FindConf(mgcp.EventNum);
			if (Room == nullptr)
			{
				loggit("room not found");
				server->reply(mgcp.ResponseBAD(400, "Room not found"), udpTO);
				return;
			}
			auto Port = GetFreePort();
			loggit("join Conf with ID=" + boost::to_string(Room->GetRoomID()) + " my_port=" + boost::to_string(Port));
			auto my_SDP = GenSDP(Port, mgcp);
			Room->NewInitPoint("", my_SDP, mgcp.paramC, Port);
			server->reply(mgcp.ResponseOK(200, mgcp.EventS) + my_SDP, udpTO);
			loggit("Conf joined");
			return;
		}
	}//case cnf

	case MGCP::prx:
	{
		if (mgcp.EventS + mgcp.EventNum == "prx/$")
		{
			auto Port = GetFreePort();
			mgcp.EventNum = boost::to_string(SetRoomID());
			loggit("creating Proxy with ID=" + mgcp.EventNum + " my_port=" + boost::to_string(Port));
			SHP_Proxy proxy(new Proxy(mgcp.SDP, Port, mgcp.EventNum, mgcp.paramC));
			ProxyVec_.push_back(proxy);

			server->reply(mgcp.ResponseOK(200, mgcp.EventS) + GenSDP(Port, mgcp), udpTO);
			loggit("Proxy created");
			return;
		}
		else
		{
			auto proxy = FindProxy(mgcp.EventNum);
			if (proxy == nullptr)
			{
				loggit("Proxy not found");
				server->reply(mgcp.ResponseBAD(400, "Proxy not found"), udpTO);
				return;
			}
			auto Port = GetFreePort();
			loggit("join Proxy with ID=" + proxy->GetID() + " my_port=" + boost::to_string(Port));
			proxy->NewPoint(mgcp.SDP, Port, mgcp.paramC);
			server->reply(mgcp.ResponseOK(200, mgcp.EventS) + GenSDP(Port, mgcp), udpTO);
			loggit("Proxy joined");
			return;
		}
	}//case MGCP::prx

	default:
	{
		server->reply(mgcp.ResponseBAD(400, "Bad Request"), udpTO);
		loggit("Bad Request");
		return;
	}//default
	}//switch
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void ConfControl::proceedMDCX(MGCP &mgcp, const udp::endpoint& udpTO)
{
	loggit(mgcp.EventEx + " MDCX for " + mgcp.paramC);
	auto Room = FindConf(mgcp.EventNum);
	if (Room == nullptr)
	{
		server->reply(mgcp.ResponseBAD(400, "Room not found"), udpTO);
		return;
	}
	auto Point = Room->FindPoint(mgcp.paramC);
	if (Point == nullptr)
	{
		server->reply(mgcp.ResponseBAD(400, "Point not found"), udpTO);
		return;
	}
	loggit("modify point with ID=" + mgcp.paramC + " in room with ID=" + boost::to_string(Room->GetRoomID()));
	auto response = Room->ModifyPoint(Point, mgcp.SDP);
	server->reply(mgcp.ResponseOK(200, "") + response, udpTO);
	loggit("point with ID=" + mgcp.paramC + " in room with ID="+boost::to_string(Room->GetRoomID())+" modified");
	return;
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void ConfControl::proceedDLCX(MGCP &mgcp, const udp::endpoint& udpTO)
{
	loggit(mgcp.EventEx + " DLCX for " + mgcp.paramC);
	switch (mgcp.Event)
	{
	case MGCP::ann:
	{
		auto Ann = FindAnn(mgcp.paramC);
		if (Ann == nullptr)
		{
			//out << "\nAnn not found";
			loggit("Ann not found");
			server->reply(mgcp.ResponseBAD(400, "Ann not found"), udpTO);
			return;
		}
		Ann->Stop();
		SetFreePort(Ann->GetPort());
		loggit("Stoped Ann with ID=" + mgcp.EventNum + " and port=" + boost::to_string(Ann->GetPort()));
		AnnVec_.erase(std::remove(AnnVec_.begin(), AnnVec_.end(), Ann), AnnVec_.end());
		RoomsID_.erase(std::remove(RoomsID_.begin(), RoomsID_.end(), stoi(mgcp.EventNum)), RoomsID_.end());
		server->reply(mgcp.ResponseOK(250, ""), udpTO);
		return;
	}//case MGCP::ann
	case MGCP::cnf:
	{
		auto Room = FindConf(mgcp.EventNum);
		if (Room == nullptr)
		{
			server->reply(mgcp.ResponseBAD(400, "Room not found"), udpTO);
			return;
		}
		auto Point = Room->FindPoint(mgcp.paramC);
		if (Point == nullptr)
		{
			server->reply(mgcp.ResponseBAD(400, "Point not found"), udpTO);
			return;
		}
		SetFreePort(Point->my_port_);
		loggit("Deleted point with ID=" + mgcp.paramC + " and port=" + boost::to_string(Point->my_port_));
		Room->DeletePoint(mgcp.paramC);
		if (Room->GetNumCllPoints() == 0)
		{
			RoomsID_.erase(std::remove(RoomsID_.begin(), RoomsID_.end(), Room->GetRoomID()), RoomsID_.end());
			RoomsVec_.erase(std::remove(RoomsVec_.begin(), RoomsVec_.end(), Room), RoomsVec_.end());
			loggit("Deleted Room with ID=" + Room->GetRoomID());
			Room.reset();
		}
		server->reply(mgcp.ResponseOK(250, ""), udpTO);
		return;
	}//case MGCP::cnf
	case MGCP::prx:
	{
		auto proxy = FindProxy(mgcp.EventNum);
		if (proxy == nullptr)
		{
			loggit("Proxy not found");
			server->reply(mgcp.ResponseBAD(400, "Proxy not found"), udpTO);
			return;
		}
		SetFreePort(proxy->DeletePoint(mgcp.paramC));
		loggit("Deleted point from Proxy with ID=" + mgcp.EventNum + " client=" + mgcp.paramC);
		if (proxy->Size() == 0)
		{
			loggit("Deleted Proxy with ID=" + mgcp.EventNum);
			ProxyVec_.erase(std::remove(ProxyVec_.begin(), ProxyVec_.end(), proxy), ProxyVec_.end());
			RoomsID_.erase(std::remove(RoomsID_.begin(), RoomsID_.end(), stoi(mgcp.EventNum)), RoomsID_.end());
		}
		server->reply(mgcp.ResponseOK(250, ""), udpTO);
		return;
	}//case MGCP::prx
	default:
	{
		server->reply(mgcp.ResponseBAD(400, "Bad Request"), udpTO);
		loggit("Bad Request");
		return;
	}//default
	}//switch
	
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void ConfControl::proceedRQNT(MGCP &mgcp, const udp::endpoint& udpTO)
{
	loggit(mgcp.EventEx + " RQNT for " + mgcp.paramC);
	auto Ann = FindAnn(mgcp.paramC);
	if (Ann == nullptr)
	{
		loggit("Ann not found");
		server->reply(mgcp.ResponseBAD(400, "Ann not found"), udpTO);
		return;
	}
	auto param = mgcp.paramS;
	string test = param.substr(param.find("file:///") + 8);
	test.pop_back();
	string filepath = server->m_args.strMmediaPath + "\\"+test;

	boost::thread my_thread(&Ann::Send, Ann, filepath);
	my_thread.detach();
	server->reply(mgcp.ResponseOK(200, ""), udpTO);
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
SHP_CConfRoom ConfControl::CreateNewRoom()
{
	/*Создаем комнату*/
	SHP_CConfRoom NewRoom(new CConfRoom());
	//out<< RoomsVec_.size();
	mutex_.lock();
	NewRoom->SetRoomID(SetRoomID());
	RoomsVec_.push_back(NewRoom);
	mutex_.unlock();
	return NewRoom;
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
int ConfControl::GetFreePort()
{

	int freeport = 29000; // базовый порт, с которого начинаем
	if (PortsinUse_.size() == 0) 
	{ 
		mutex_.lock();
		PortsinUse_.push_back(freeport); 
		std::sort(PortsinUse_.begin(), PortsinUse_.end());
		mutex_.unlock();
		return freeport;
	}
	for (unsigned i = 0; i < PortsinUse_.size(); ++i)
	{
		if (PortsinUse_[i] != freeport)
		{ 
			mutex_.lock();
			PortsinUse_.push_back(freeport);
			std::sort(PortsinUse_.begin(), PortsinUse_.end());
			mutex_.unlock();
			return freeport;
		}
		freeport += 2;
	}
	mutex_.lock();
	PortsinUse_.push_back(freeport);
	std::sort(PortsinUse_.begin(), PortsinUse_.end());
	mutex_.unlock();
	return freeport;
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void ConfControl::SetFreePort(int port)
{
	mutex_.lock();
	PortsinUse_.erase(std::remove(PortsinUse_.begin(), PortsinUse_.end(), port),
		PortsinUse_.end()); // удаляем порт из занятых
	mutex_.unlock();
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
SHP_CConfRoom ConfControl::FindConf(string ID)
{
	for (auto &room : RoomsVec_)
	{ 
		if (ID == boost::to_string(room->GetRoomID()))
			return room;
	}
	return nullptr;
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
SHP_Ann ConfControl::FindAnn(string ID)
{
	for (auto &ann : AnnVec_)
	{
		if (ID == ann->CallID_)
			return ann;
	}
	return nullptr;
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
SHP_Proxy ConfControl::FindProxy(string ID)
{
	for (auto &proxy : ProxyVec_)
	{
		if (ID == proxy->GetID())
			return proxy;
	}
	return nullptr;
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
int ConfControl::SDPFindMode(string SDP)
{
	std::size_t found;
	found = SDP.find("inactive");
	if (found != std::string::npos)
		return 0;
	found = SDP.find("sendrecv");
	if (found != std::string::npos)
		return 1;
	return -1;
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------