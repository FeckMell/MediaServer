#pragma once
//#include "PackMGCP.h"
#include "Connection.h"
#include "MGCPparser.h"
#include "conf.h"

//struct TMGCP;

/************************************************************************
	CMGCPServer
************************************************************************/
class CMGCPConnection;
typedef MGCP::TEndPoint KEY_MGCPConnection;
typedef shared_ptr<CMGCPConnection> SHP_CMGCPConnection;
typedef std::map<KEY_MGCPConnection, SHP_CMGCPConnection> MAP_CMGCPConnections;
//typedef shared_ptr<CMGCPConnection> SHP_CMGCPConnection;

class CMGCPServer
{
public:
	struct TArgs
	{
		asio::io_service&		io_service; 
		const udp::endpoint&	endpnt;
		string					strMmediaPath;
		asio::io_service&		io_service1;

	};
	struct ConfParam 
	{
		bool operator ==(const ConfParam &compare) const //перегрузка оператора сравнения
		{
			return CallID == compare.CallID;
		}
		TRTP_Dest PortsAndAddr;
		string sdpIn;
		string CallID;
		int PortAsio;
		string SDPresponse;
		int error = 0;


	};
	CMGCPServer(const TArgs&, FILE * FileLogS);
	const udp::endpoint& EndP_Local() const { return m_args.endpnt; }
	void Run();
private:
	void do_receive();
	void do_send(std::size_t length);
	void respond(const string);
	// просиды
	void proceedError(int ErrorCode); // обработчик ошибок
	void proceedReceiveBuffer(const char*, const udp::endpoint&);
	void proceedDLCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO);
	void proceedRQNT(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO);
	void proceedCRCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO);
	void proceedMDCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO);
	void reply(const string&, const udp::endpoint&);
	// соединение
	SHP_CMGCPConnection getConnection(const MGCP::TMGCP&, const udp::endpoint&);
	SHP_CMGCPConnection findConnection(const MGCP::TMGCP& mgcp) const;
	ConfParam FillinConfParam(MGCP::TMGCP &mgcp, int mode); // заполняем порты, адреса и SDP для конфы
	void Connectivity(MGCP::TMGCP &mgcp); // выполняем стандартные действия подключения
	// порты
	void SetFreePort(SHP_CConfRoom room, string CallID); // освобождаем сокет
	bool FindPort(int sc); // узнаем, занят ли сокет
	int GetFreePort(); // получаем номер свободного сокета
	// комната
	//bool CompareRooms(SHP_CConfRoom i, SHP_CConfRoom j);// сравнение комнат
	SHP_CConfRoom FindRoom(int ID);//находим комнату по ИД
	SHP_CConfRoom CreateNewRoom(/*asio::io_service& io_service*/);// создание комнаты
	void DeleteRoom(SHP_CConfRoom remRoom); // удаление комнаты
	string GetRoomIDConn(string s);// узнать к какой комнате обращаются по ИД
	//bool CheckExistence(string CallID); // узнаем, подключен ли этот CallId к этой комнате

	TArgs	m_args;
	udp::socket socket_;
	udp::endpoint sender_endpoint_;
	enum { max_length = 2048 };
	char data_[max_length + 1];

	MAP_CMGCPConnections m_cllConnections;
	//std::vector<make_shared<CMGCPConnection>> shpConnect_;
	std::vector<SHP_CConfRoom> RoomsVec_; // вектор существующих комнат
	std::vector<int> PortsinUse_; // вектор занятых портов
	FILE * file;// для записи логов
	std::vector<ConfParam> SDPforCRCX_;// вектор объявленных соединений
	std::mutex  mutex_;
	asio::io_service& io_service__;
	FILE *FileLogS;
	//boost::scoped_ptr<std::thread> ThreadAddPoint;

};
