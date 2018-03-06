#pragma once
#include "stdafx.h"
#include "ConfControl.h"
void SendAnn(SHP_Ann ann, string file)
{
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
void ConfControl::proceedCRCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	if (mgcp.EndPoint.m_point == "ann/$")
	{
		auto Port = GetFreePort();
		SHP_Ann Ann(new Ann(mgcp.SDP, Port, mgcp.getCallID(), SetRoomID()));
		AnnVec_.push_back(Ann);

		auto f = boost::format(string("\n\nv=0\no=- %3% 0 IN IP4 %1%\ns=%4%\nc=IN IP4 %1%\nt=0 0\na=tool:libavformat 57.3.100\nm=audio %2% RTP/AVP 8\na=rtpmap:8 PCMA/8000\na=ptime:20\na=sendrecv\n")); // формируем тип ответа
		auto my_SDP = str(f %my_IP %Port % (rand() % 100000) % (rand() % 100000));
		server->reply(mgcp.ResponseOK() + my_SDP, udpTO);
	}

	if (mgcp.EndPoint.m_point.substr(0, 4) != "cnf/")
	{
		server->loggit("!= cnf / ");
		return;
	}

	if (mgcp.EndPoint.m_point == "cnf/$")
	{
		cout << "\n1";
		auto Room = CreateNewRoom();
		cout << "\n2";
		auto Port = GetFreePort();
		cout << "\n3";
		auto f = boost::format(string("\n\nv=0\no=- %3% 0 IN IP4 %1%\ns=%4%\nc=IN IP4 %1%\nt=0 0\na=tool:libavformat 57.3.100\nm=audio %2% RTP/AVP 8\na=rtpmap:8 PCMA/8000\na=ptime:20\na=sendrecv\n")); // формируем тип ответа
		auto my_SDP = str(f %my_IP %Port %(rand() % 100000) %(rand() % 100000));
		cout << "\n4";
		Room->NewInitPoint(mgcp.SDP, my_SDP, mgcp.getCallID(), Port);
		cout << "\n5";
		server->reply(mgcp.NewResponseOK(200,mgcp.EndPoint.m_point.substr(0, 4)+std::to_string(Room->GetRoomID())) + my_SDP, udpTO);
	}
	else
	{
		auto ConfNum = mgcp.EndPoint.m_point.substr(4);
		server->loggit("cnf/"+ConfNum);
		auto Room = FindRoom(ConfNum);
		if (Room == nullptr)
		{
			server->loggit("room not found");
			server->reply(mgcp.ResponseBAD(400) + "Room not found", udpTO);
			return;
		}
		auto Port = GetFreePort();
		auto f = boost::format(string("\n\nv=0\no=- %3% 0 IN IP4 %1%\ns=%4%\nc=IN IP4 %1%\nt=0 0\na=tool:libavformat 57.3.100\nm=audio %2% RTP/AVP 8\na=rtpmap:8 PCMA/8000\na=ptime:20\na=sendrecv\n")); // формируем тип ответа
		auto my_SDP = str(f %my_IP %Port % (rand() % 100000) % (rand() % 100000));
		Room->NewInitPoint("", my_SDP, mgcp.getCallID(), Port);
		server->reply(mgcp.NewResponseOK(200, mgcp.EndPoint.m_point) + my_SDP, udpTO);
	}
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void ConfControl::proceedMDCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	auto ConfNum = mgcp.EndPoint.m_point.substr(4);
	auto Room = FindRoom(ConfNum);
	if (Room == nullptr)
	{
		server->reply(mgcp.ResponseBAD(400) + "Room not found", udpTO);
		return;
	}
	auto Point = Room->FindPoint(mgcp.getCallID());
	if (Point == nullptr)
	{
		server->reply(mgcp.ResponseBAD(400) + "Point not found", udpTO);
		return;
	}
	cout << "\nSDPServer\n" << mgcp.SDP;
	auto response = Room->ModifyPoint(Point, mgcp.SDP);
	server->reply(mgcp.ResponseOK() + response, udpTO);
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void ConfControl::proceedDLCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	auto ConfNum = mgcp.EndPoint.m_point.substr(4);
	auto Room = FindRoom(ConfNum);
	if (Room == nullptr)
	{
		server->reply(mgcp.ResponseBAD(400) + "Room not found", udpTO);
		return;
	}
	auto Point = Room->FindPoint(mgcp.getCallID());
	if (Point == nullptr) 
	{
		server->reply(mgcp.ResponseBAD(400) + "Room not found", udpTO);
		return;
	}
	SetFreePort(Point->my_port_);
	Room->DeletePoint(mgcp.getCallID());
	if (Room->GetNumCllPoints() == 0)
	{
		RoomsID_.erase(std::remove(RoomsID_.begin(), RoomsID_.end(), Room->GetRoomID()), RoomsID_.end());
		RoomsVec_.erase(std::remove(RoomsVec_.begin(), RoomsVec_.end(), Room), RoomsVec_.end());
		Room.reset();
	}
	server->reply(mgcp.ResponseOK(250), udpTO);
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void ConfControl::proceedRQNT(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	auto Ann = FindAnn(mgcp.getCallID());
	auto param = mgcp.getSignalParam();
	string filepath = param->m_value.substr(param->m_value.find("file:///") + 7, param->m_value.find(")\n") - (param->m_value.find("file:///") + 7));
	boost::thread my_thread(&SendAnn, Ann, filepath);
	my_thread.detach();
	server->reply(mgcp.ResponseOK(), udpTO);
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
SHP_CConfRoom ConfControl::CreateNewRoom()
{
	/*Создаем комнату*/
	SHP_CConfRoom NewRoom(new CConfRoom());
	cout<< RoomsVec_.size();
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