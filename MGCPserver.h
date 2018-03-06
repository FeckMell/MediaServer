#pragma once
//#include "PackMGCP.h"
#include "Connection.h"
#include "MGCPparser.h"
#include "Conf.h"
//#include "DestFusion.h"
//struct TMGCP;
extern FILE *FileLogServer;
/************************************************************************
	CMGCPServer
************************************************************************/
class CMGCPConnection;
typedef MGCP::TEndPoint KEY_MGCPConnection;
typedef shared_ptr<CMGCPConnection> SHP_CMGCPConnection;
typedef std::map<KEY_MGCPConnection, SHP_CMGCPConnection> MAP_CMGCPConnections;
//typedef shared_ptr<CMGCPConnection> SHP_CMGCPConnection;

struct ConfParam
{
	bool operator ==(const ConfParam &compare) const //перегрузка оператора сравнени€
	{
		return CallID == compare.CallID;
	}
	int my_port;
	string input_SDP;
	string CallID;
	string SDPresponse;
	int error = 0;
};
typedef shared_ptr<ConfParam> SHP_ConfParam;
class CMGCPServer
{
public:
	typedef std::lock_guard<std::mutex> lock;
	struct TArgs
	{
		asio::io_service&		io_service; 
		const udp::endpoint&	endpnt;
		string					strMmediaPath;
		asio::io_service&		io_service1;

	};
	
	CMGCPServer(const TArgs&);
	const udp::endpoint& EndP_Local() const { return m_args.endpnt; }
	void Run();
private:
	void loggit(string a);
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
	SHP_ConfParam FillinConfParam(MGCP::TMGCP &mgcp, int mode); // заполн€ем порты, адреса и SDP дл€ конфы
	void Connectivity(MGCP::TMGCP &mgcp); // выполн€ем стандартные действи€ подключени€
	// порты
	void SetFreePort(SHP_CConfRoom room, string CallID); // освобождаем сокет
	bool FindPort(int sc); // узнаем, зан€т ли сокет
	int GetFreePort(); // получаем номер свободного сокета
	// комната
	//bool CompareRooms(SHP_CConfRoom i, SHP_CConfRoom j);// сравнение комнат
	SHP_CConfRoom FindRoom(int ID);//находим комнату по »ƒ
	SHP_ConfParam FindClient(string CallID);
	SHP_CConfRoom CreateNewRoom(/*asio::io_service& io_service*/);// создание комнаты
	void DeleteRoom(SHP_CConfRoom remRoom); // удаление комнаты
	string GetRoomIDConn(string s);// узнать к какой комнате обращаютс€ по »ƒ
	//bool CheckExistence(string CallID); // узнаем, подключен ли этот CallId к этой комнате
	string DeleteFromSDP(string inputSDP, int my_port);

	TArgs	m_args;
	udp::socket socket_;
	udp::endpoint sender_endpoint_;
	enum { max_length = 2048 };
	char data_[max_length + 1];

	MAP_CMGCPConnections m_cllConnections;
	//std::vector<make_shared<CMGCPConnection>> shpConnect_;
	std::vector<SHP_CConfRoom> RoomsVec_; // вектор существующих комнат
	std::vector<int> PortsinUse_; // вектор зан€тых портов
	std::vector<SHP_ConfParam> SDPforCRCX_;// вектор объ€вленных соединений
	std::mutex  mutex_;
	asio::io_service& io_service__;
	
	//boost::scoped_ptr<std::thread> ThreadAddPoint;

};

