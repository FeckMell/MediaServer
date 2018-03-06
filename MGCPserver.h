#pragma once
//#include "PackMGCP.h"
#include "stdafx.h"
#include "ConfControl.h"
#include "Functions.h"
#include "Connection.h"
#include "MGCPparser.h"
#include "Conf.h"
//#include "DestFusion.h"
//struct TMGCP;
/************************************************************************
	CMGCPServer
************************************************************************/
class ConfControl;
//class CMGCPServer;
class CMGCPConnection;
typedef MGCP::TEndPoint KEY_MGCPConnection;
typedef shared_ptr<CMGCPConnection> SHP_CMGCPConnection;
typedef std::map<KEY_MGCPConnection, SHP_CMGCPConnection> MAP_CMGCPConnections;
//typedef shared_ptr<CMGCPConnection> SHP_CMGCPConnection;
extern FILE *FileLogServer;

struct ConfParam
{
	/*Для поиска параметров соединения по CallID*/
	bool operator ==(const ConfParam &compare) const
	{
		return CallID == compare.CallID;
	}
	/*Информация*/
	int my_port;
	int remote_port;
	int repote_IP;
	string input_SDP;
	string CallID;
	string SDPresponse;
	int error = 0;
};
typedef shared_ptr<ConfParam> SHP_ConfParam;

class CMGCPServer
{
public:
	//SHP_ConfControl a;
	ConfControl* Conference;
	typedef std::lock_guard<std::mutex> lock;
	struct TArgs
	{
		asio::io_service&		io_service; 
		const udp::endpoint&	endpnt;
		string					strMmediaPath;
	};
	
	CMGCPServer(const TArgs&);
	const udp::endpoint& EndP_Local() const { return m_args.endpnt; }
	void Run();
	void reply(const string&, const udp::endpoint&);
	void loggit(string a);
private:
/*Отладка*/
	//void loggit(string a);

	void do_receive();
	void do_send(std::size_t length);
	void respond(const string);
	//void reply(const string&, const udp::endpoint&);

/*Первичная обработка команд*/
	void proceedReceiveBuffer(const char*, const udp::endpoint&);
	void proceedDLCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO);
	void proceedRQNT(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO);
	void proceedCRCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO);
	void proceedMDCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO);

/*Вторичная обработка команд*/
	void proceedCRCX_CNF_0(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO);
	void proceedCRCX_CNF_N(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO);
	void proceedCRCX_ANN_0(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO);
	
/*Соединение*/
	SHP_CMGCPConnection getConnection(const MGCP::TMGCP&, const udp::endpoint&);
	SHP_CMGCPConnection findConnection(const MGCP::TMGCP& mgcp) const;
	void Connectivity(MGCP::TMGCP &mgcp); // выполняем стандартные действия подключения

/* Работа с портами*/
	/*Освобождаем сокет*/
	void SetFreePort(int port);
	/*Узнаем, занят ли сокет*/
	bool FindPort(int sc);
	/*Получаем номер свободного*/
	int GetFreePort(); // получаем номер свободного сокета

/* Работа с комнатой*/
	/*Находим комнату по ID*/
	SHP_CConfRoom FindRoom(int ID);
	/* Находим сохраненный поинт из CNF_N */
	SHP_ConfParam FindClient(string CallID);
	/* Создаем новую комнату*/
	SHP_CConfRoom CreateNewRoom();

/* Работа с информацией*/
	/* Находим N из CNF_N*/
	string GetRoomIDConn(string s);
	/*Создание и заполнение структуры данных для конфы*/
	SHP_ConfParam FillinConfParam(MGCP::TMGCP &mgcp);
	/*Удаление из SDP клиента лишей информации*/
	string DeleteFromSDP(string inputSDP, int my_port);
	/*Поиск типа соединения(inactive/sendrecv) в SDP*/
	int SDPFindMode(string SDP);
	/*Смена в SDP типа соединения*/
	string ChangeSDPMode(string SDP);

	TArgs	m_args;
	udp::socket socket_;
	udp::endpoint sender_endpoint_;
	enum { max_length = 2048 };
	char data_[max_length + 1];

	MAP_CMGCPConnections m_cllConnections;
	//std::vector<make_shared<CMGCPConnection>> shpConnect_;
	std::vector<SHP_CConfRoom> RoomsVec_; // вектор существующих комнат
	std::vector<int> PortsinUse_; // вектор занятых портов
	std::vector<SHP_ConfParam> SDPforCRCX_;// вектор объявленных соединений
	std::mutex  mutex_;
	asio::io_service& io_service__;
	
	
	//boost::scoped_ptr<std::thread> ThreadAddPoint;

};

