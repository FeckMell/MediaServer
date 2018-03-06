#pragma once
#include "stdafx.h"
#include "ConfControl.h"
void SendAnn(SHP_Ann ann, string file)
{
	//out << "\nthread SendAnn " << std::this_thread::get_id();
	ann->Send(file);
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
void ConfControl::proceedCRCX(MGCP &mgcp, const udp::endpoint& udpTO)
{
	countCRCX++;
	cout << "CRCX " << countCRCX << "\n";
	if (mgcp.Event+mgcp.EventNum == "ann/$")
	{
		server->loggit(mgcp.EventEx);
		auto Port = GetFreePort();
		SHP_Ann Ann(new Ann(mgcp.SDP, Port, mgcp.paramC));
		AnnVec_.push_back(Ann);
		mgcp.EventNum = std::to_string(SetRoomID());

		auto f = boost::format(string("\n\nv=0\no=- %3% 0 IN IP4 %1%\ns=%4%\nc=IN IP4 %1%\nt=0 0\na=tool:libavformat 57.3.100\nm=audio %2% RTP/AVP 8\na=rtpmap:8 PCMA/8000\na=ptime:20\na=sendrecv\n")); // формируем тип ответа
		auto my_SDP = str(f %my_IP %Port % (rand() % 100000) % (rand() % 100000));
		server->reply(mgcp.ResponseOK(200, mgcp.Event) + my_SDP, udpTO);
		return;
		//сделать отправку номера и тут и в cnf
	}

	if (mgcp.Event != "cnf/")
	{
		server->loggit(mgcp.EventEx);
		server->loggit("!= cnf / ");
		return;
	}

	if (mgcp.Event + mgcp.EventNum == "cnf/$")
	{
		server->loggit(mgcp.EventEx);
		auto Room = CreateNewRoom();
		auto Port = GetFreePort();
		auto f = boost::format(string("\n\nv=0\no=- %3% 0 IN IP4 %1%\ns=%4%\nc=IN IP4 %1%\nt=0 0\na=tool:libavformat 57.3.100\nm=audio %2% RTP/AVP 8\na=rtpmap:8 PCMA/8000\na=ptime:20\na=sendrecv\n")); // формируем тип ответа
		auto my_SDP = str(f %my_IP %Port %(rand() % 100000) %(rand() % 100000));
		Room->NewInitPoint(mgcp.SDP, my_SDP, mgcp.paramC, Port);
		mgcp.EventNum = std::to_string(Room->GetRoomID());
		server->reply(mgcp.ResponseOK(200, mgcp.Event) + my_SDP, udpTO);
		return;
	}
	else
	{
		server->loggit(mgcp.EventEx);
		auto Room = FindRoom(mgcp.EventNum);
		if (Room == nullptr)
		{
			server->loggit("room not found");
			server->reply(mgcp.ResponseBAD(400, "Room not found"), udpTO);
			return;
		}
		auto Port = GetFreePort();
		auto f = boost::format(string("\n\nv=0\no=- %3% 0 IN IP4 %1%\ns=%4%\nc=IN IP4 %1%\nt=0 0\na=tool:libavformat 57.3.100\nm=audio %2% RTP/AVP 8\na=rtpmap:8 PCMA/8000\na=ptime:20\na=sendrecv\n")); // формируем тип ответа
		auto my_SDP = str(f %my_IP %Port % (rand() % 100000) % (rand() % 100000));
		Room->NewInitPoint("", my_SDP, mgcp.paramC, Port);
		server->reply(mgcp.ResponseOK(200, mgcp.Event) + my_SDP, udpTO);
		return;
	}
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void ConfControl::proceedMDCX(MGCP &mgcp, const udp::endpoint& udpTO)
{
	server->loggit(mgcp.EventEx);
	auto Room = FindRoom(mgcp.EventNum);
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
	auto response = Room->ModifyPoint(Point, mgcp.SDP);
	server->reply(mgcp.ResponseOK(200, "") + response, udpTO);
	return;
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void ConfControl::proceedDLCX(MGCP &mgcp, const udp::endpoint& udpTO)
{
	//out << "\nthread DLCX " << std::this_thread::get_id();
	if (mgcp.Event == "ann/")
	{
		server->loggit(mgcp.EventEx);
		auto Ann = FindAnn(mgcp.paramC);
		if (Ann == nullptr)
		{
			//out << "\nAnn not found";
			server->loggit("Ann not found");
			server->reply(mgcp.ResponseBAD(400, "Ann not found"), udpTO);
			return;
		}
		Ann->Stop();
		SetFreePort(Ann->GetPort());
		AnnVec_.erase(std::remove(AnnVec_.begin(), AnnVec_.end(), Ann), AnnVec_.end());
		RoomsID_.erase(std::remove(RoomsID_.begin(), RoomsID_.end(), stoi(mgcp.EventNum)), RoomsID_.end());
		server->reply(mgcp.ResponseOK(250, ""), udpTO);
	}
	else if (mgcp.Event == "cnf/")
	{
		server->loggit(mgcp.EventEx);
		auto Room = FindRoom(mgcp.EventNum);
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
		Room->DeletePoint(mgcp.paramC);
		if (Room->GetNumCllPoints() == 0)
		{
			RoomsID_.erase(std::remove(RoomsID_.begin(), RoomsID_.end(), Room->GetRoomID()), RoomsID_.end());
			RoomsVec_.erase(std::remove(RoomsVec_.begin(), RoomsVec_.end(), Room), RoomsVec_.end());
			Room.reset();
		}
		server->reply(mgcp.ResponseOK(250, ""), udpTO);
		return;
	}
	
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void ConfControl::proceedRQNT(MGCP &mgcp, const udp::endpoint& udpTO)
{
	auto Ann = FindAnn(mgcp.paramC);
	if (Ann == nullptr)
	{
		//out << "\nAnn not found";
		server->loggit("Ann not found");
		server->reply(mgcp.ResponseBAD(400, "Ann not found"), udpTO);
		return;
	}
	auto param = mgcp.paramS;
	string test = param.substr(param.find("file:///") + 8);
	test.pop_back();
	string filepath = server->m_args.strMmediaPath + "\\"+test;
	boost::thread my_thread(&SendAnn, Ann, filepath);
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
	NewRoom->SetRoomID(SetRoomID());
	RoomsVec_.push_back(NewRoom);
	return NewRoom;
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
int ConfControl::GetFreePort()
{

	int freeport = 29000; // базовый порт, с которого начинаем
	if (PortsinUse_.size() == 0) { PortsinUse_.push_back(freeport); return freeport; }
	for (unsigned i = 0; i < PortsinUse_.size(); ++i)
	{
		if (PortsinUse_[i] != freeport){ PortsinUse_.push_back(freeport); return freeport; }
		freeport += 2;
	}
	PortsinUse_.push_back(freeport);
	std::sort(PortsinUse_.begin(), PortsinUse_.end());
	return freeport;
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void ConfControl::SetFreePort(int port)
{
	PortsinUse_.erase(std::remove(PortsinUse_.begin(), PortsinUse_.end(), port),
		PortsinUse_.end()); // удаляем порт из занятых
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
SHP_CConfRoom ConfControl::FindRoom(string ID)
{
	for (auto &room : RoomsVec_)
	{ 
		if (ID == std::to_string(room->GetRoomID()))
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
		//out << "\nID=" + ID + "?=" + ann->CallID_;
		if (ID == ann->CallID_)
			return ann;
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