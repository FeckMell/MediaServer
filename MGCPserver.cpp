#include "stdafx.h"
#include "MGCPserver.h"
#include "Utils.h"
//#include <string>
//#include <iostream> 
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstddef>
/*
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
*/
#include <boost/algorithm/string.hpp>
#include <boost/scope_exit.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <thread>
//#define BOOST_REGEX_MATCH_EXTRA
//#include <boost/regex.hpp>
/*
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/find_iterator.hpp>

*/
namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;
//FILE *FileLogS;
template <typename Iterator>
bool parse_filename(Iterator first, Iterator last, string& strOut)
{
	using qi::char_;

	qi::rule<Iterator, string()> _file;
	_file = *(char_ - char_("()"));

	bool b = qi::parse(first, last,
		*(*(char_ - ',') >> ',') >> "file://"
		>> _file[phoenix::ref(strOut) = qi::_1]
		);

	//b = first == last;
	return b;
}

/*boost::asio::ip::udp::endpoint getSelfIP()
{
	asio::io_service io_service;
	const auto strHostName = asio::ip::host_name();

	boost::system::error_code ec;
	udp::resolver resolver(io_service);
	udp::resolver::query query(udp::v4(), strHostName, "0");
	udp::resolver::iterator const end, itr = resolver.resolve(query, ec);
	if (ec)
		cout << "Error resolving myself: " << ec.message() << '\n';
	assert(itr != end);

	return itr->endpoint()/*.address()/*.to_string();
}*/
static ThreadedSet<unsigned> thsetAnnIdInUse;
static ThreadedSet<unsigned> thsetCnfIdInUse;

/************************************************************************
	CMGCPServer
************************************************************************/
CMGCPServer::CMGCPServer(const TArgs& args , FILE * FileLog)
: m_args(args), io_service__(args.io_service1), socket_(args.io_service, udp::endpoint(udp::v4(), args.endpnt.port())), FileLogS(FileLog)
{
	fprintf(FileLogS, "Server Construct\n");
	fopen_s(&FileLogS, "LOGS_Server.txt", "w");
}

//-----------------------------------------------------------------------
void CMGCPServer::do_receive()
{
	socket_.async_receive_from(
		asio::buffer(data_, max_length), sender_endpoint_,
		[this](boost::system::error_code ec, std::size_t bytes_recvd)
	{
		/*
		if (!ec && bytes_recvd > 0)
		do_send(bytes_recvd);
		else
		do_receive();
		*/
		data_[bytes_recvd] = 0;
		string strTmp(data_);
		boost::replace_all(strTmp, "\n", "!\n");
		boost::replace_all(strTmp, "\r!", "!");
		fprintf(FileLogS, "---------  sent---------\n");
		printf("--------- %s sent:\n%s\n--------- len %lu ---------\n\n",
			sender_endpoint_.address().to_string().c_str(), strTmp.c_str(), bytes_recvd);
		proceedReceiveBuffer(data_, sender_endpoint_);
	});
}

//-----------------------------------------------------------------------
void CMGCPServer::do_send(std::size_t length)
{
	//printf("\ndo_send\n");
	socket_.async_send_to(
		asio::buffer(data_, length), sender_endpoint_,
		[this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/)
	{
		do_receive();
	});
}

//-----------------------------------------------------------------------
SHP_CMGCPConnection CMGCPServer::findConnection(const MGCP::TMGCP& mgcp) const
{
	auto itr = m_cllConnections.find(mgcp.EndPoint);
	if (itr == m_cllConnections.cend())
		return SHP_CMGCPConnection();

	return itr->second;
}

//-----------------------------------------------------------------------
SHP_CMGCPConnection CMGCPServer::getConnection(const MGCP::TMGCP& mgcp, const udp::endpoint& udpTO)
{
	auto conn = findConnection(mgcp);
	if (!conn)
		//error 500 The transaction could not be executed, because the endpoint is unknown.
		reply(str(boost::format("500 %1% unknown endpoint") % mgcp.IdTransact), udpTO);

	return conn;
}

#define DEBUG___
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
void threadSendMedia(SHP_CMGCPConnection conn, string strFile, string CallID)
{	
	//printf("\n test333\n");
	//auto CallID = mgcp.getCallID();
	conn->sendMedia(strFile, CallID);
}

//-----------------------------------------------------------------------
void CMGCPServer::proceedRQNT(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	fprintf(FileLogS, "RQNT\n");
	assert(mgcp.CMD == MGCP::TMGCP::RQNT);
	SHP_CMGCPConnection conn = getConnection(mgcp, udpTO);
	if (conn)
	{
		reply(mgcp.ResponseOK(), udpTO);

		if (auto param = mgcp.getSignalParam())
		{
			string strFile;
			if(parse_filename(param->m_value.cbegin(), param->m_value.cend(), strFile))
			{
				//TODO: multithreading & concurrent issue
				auto CallID = mgcp.getCallID();
				std::thread th(threadSendMedia, conn, m_args.strMmediaPath + strFile, CallID);
				th.detach();
				fwrite("RQNT done\n", strlen("RQNT done\n"), 1, file);
/*
				non-stable in UNIX
				std::thread th([&](SHP_CMGCPConnection c)
				{
				c->sendMedia(m_args.strMmediaPath + strFile);
				},
				conn);*/
/*
				conn->setMediaFile(m_args.strMmediaPath + strFile);
				std::thread th([](SHP_CMGCPConnection c){c->sendMedia(); }, conn);
				th.detach();*/

			}
		}
	}
}

//-----------------------------------------------------------------------
void CMGCPServer::proceedReceiveBuffer(const char* pCh, const udp::endpoint& udpTO)
{
	using namespace MGCP;
	const auto start_tp = chrono::high_resolution_clock::now();
	//cout << "\n==============recieved=================\n";
	fprintf(FileLogS, "\n==============recieved=================\n");
	cout << boost::format("=================== %1% sent:\n%2%\n===================\n") % udpTO % pCh; // верхняя строчка короче
	TMGCP mgcp;
	if (!parseMGCP(pCh, mgcp))
	{
		reply("Not MGCP", udpTO);
		//socket_.send_to(asio::buffer("Not MGCP"), udpTO);
		return;
	}
	//cout << mgcp.CMD << "\n"; //enum MGCPCmd { NON, EPCF, CRCX, MDCX, DLCX, RQNT, NTFY, AUEP, AUCX, RSIP };

/*
	const char *pLine = strpbrk(pCh, "\n");
	if (!pLine)
		pLine = pCh;
	cout << boost::format("%1% sent: %2%...\n") % udpTO %  string(pCh, pLine);
*/


	const auto end_tp = chrono::high_resolution_clock::now();
	const auto dur = chrono::duration_cast<chrono::microseconds>(end_tp - start_tp);
	fprintf(FileLogS, "Parsing duration: in microseconds\n");
	cout << boost::format("Parsing duration: %1% microseconds\n") % dur.count();
	switch (mgcp.CMD)
	{
	case TMGCP::CRCX:
		proceedCRCX(mgcp, udpTO);
		break;
	case TMGCP::RQNT:
		proceedRQNT(mgcp, udpTO);
		break;
	case TMGCP::DLCX:
		proceedDLCX(mgcp, udpTO);
		break;
	case TMGCP::MDCX:
		proceedMDCX(mgcp, udpTO);
		break;
	default:
		//error 504 Unknown or unsupported command.
		reply(str(boost::format("504 %1% unsupported command") % mgcp.IdTransact), udpTO);
		break;
	}
}

//-----------------------------------------------------------------------
void CMGCPServer::reply(const string& str, const udp::endpoint& udpTO)
{
	socket_.send_to(asio::buffer(str), udpTO);
	fprintf(FileLogS, "\n --------------------Server REPLIED------------------------\n");
	cout << "\n --------------------Server REPLIED------------------------\n";
	//cout << boost::format("----------REPLIED TO %1%:\n%2%\n------------------\n")
		//% udpTO % str;
}

//-----------------------------------------------------------------------
void CMGCPServer::respond(const string str)
{
	//printf("\nrespond\n");
	socket_.async_send_to(
		asio::buffer(str), sender_endpoint_,
		[this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/)
	{
		do_receive();
	});
}

//-----------------------------------------------------------------------
void CMGCPServer::Run()
{
	fprintf(FileLogS, "Server.Run()");
	for (;;)
	{
		udp::endpoint sender_endpoint;
		fprintf(FileLogS, "Waiting for Callagent request...\n");
		cout << "Waiting for Callagent request...\n";

		size_t bytes_recvd = socket_.receive_from(
			asio::buffer(data_, max_length), sender_endpoint);//

		data_[bytes_recvd] = 0;//

#ifdef _DEBUG
		if (strcmp(data_, "exit") == 0)
		{
			reply("Bye!", sender_endpoint);
			break;//
		}
#endif // _DEBUG

		const char* pLogData = data_;
		proceedReceiveBuffer(data_, sender_endpoint);//
	}
}
//-----------------------------------------------------------------------
bool CMGCPServer::FindPort(int sc)
{
	for (auto &e : PortsinUse_)
		if (e == sc) return true;
	return false;
}
//-----------------------------------------------------------------------
int CMGCPServer::GetFreePort()
{
	int freeport = 29000; // базовый порт, с которого начинаем
	lock lk(mutex_);
	if (PortsinUse_.size() == 0) // если занятых портов нет
	{ 
		PortsinUse_.push_back(freeport); // добавляем новый занятый порт (16000)
		return freeport; // возвращаем порт для использования
	}
	for (unsigned i = 0; i < PortsinUse_.size(); ++i) // поиск по вектору занятых портов
	{
		if (FindPort(freeport)) // если в векторе портов есть такой
			freeport += 2; // тогда смещаемся на 2
		else
		{
			PortsinUse_.push_back(freeport);// иначе добавляем его в занятые порты
			return freeport; // и возвращаем для использования
		}
	}
	PortsinUse_.push_back(freeport); // если дошли до конца вектора, то добавляем в него порт на 2 больше максимального.
	return freeport;// возвращаем для использования
}
//-----------------------------------------------------------------------
void CMGCPServer::SetFreePort(SHP_CConfRoom room, string CallID)
{
	PortsinUse_.erase(std::remove(PortsinUse_.begin(), PortsinUse_.end(), room->FindPoint(CallID)->GetPort()), 
		PortsinUse_.end()); // удаляем порт из занятых
}
//-----------------------------------------------------------------------
/*bool CheckExistence(string CallID)
{

}*/
//-----------------------------------------------------------------------
string CMGCPServer::GetRoomIDConn(string s)
{
	string result = "";
	lock lk(mutex_);
	for (unsigned i = 4; i < s.size(); ++i)
		result += s[i];
	return result;
}
//-----------------------------------------------------------------------
SHP_CConfRoom CMGCPServer::FindRoom(int ID)
{
	auto it = std::find_if(RoomsVec_.begin(),
		RoomsVec_.end(),
		[ID](const SHP_CConfRoom& current) { return current->GetRoomID() == ID; }
	);
	if (it != RoomsVec_.end()) return *it;
	else return nullptr;
}
//-----------------------------------------------------------------------
bool CompareRooms(SHP_CConfRoom i, SHP_CConfRoom j){ return (i->GetRoomID()<j->GetRoomID()); }
//-----------------------------------------------------------------------
void CMGCPServer::DeleteRoom(SHP_CConfRoom remRoom)
{
	for (auto& point : remRoom->GetIDPoints()) // для всех поинтов освобождаем сокеты
		SetFreePort(remRoom, point);
	RoomsVec_.erase(std::remove(RoomsVec_.begin(), RoomsVec_.end(), remRoom), RoomsVec_.end()); // удаляем комнату
	// надо бы не только из вектора удалить, а вообще. И все поинты
}
//-----------------------------------------------------------------------
SHP_CConfRoom CMGCPServer::CreateNewRoom(/*asio::io_service& io_service*/)
{
	SHP_CConfRoom NewRoom(new CConfRoom(io_service__)); // создаем комнату
	unsigned int i; // предполагаемый номер комнаты
	if (RoomsVec_.size() == 0) //если комнат до этого не было
	{
		NewRoom->SetRoomID(0);// задаем ей ID
		RoomsVec_.push_back(NewRoom); // добавляем в вектор комнат
		return NewRoom;
	}
	std::vector<int> copyID; // создаем вектор ID комнат. При сортировке RoomsVec_ - копирование, что плохо (data race).
	for (auto& e : RoomsVec_) // поэтому заполняем дублированный
		copyID.push_back(e->GetRoomID());
	std::sort(copyID.begin(), copyID.end());// сортируем его, а тут пофиг на копирование
	for (i = 0; i < /*RoomsVec_*/copyID.size(); ++i) // ищем ближайший незанятый номер
		if (i != /*RoomsVec_*/copyID[i]/*->GetRoomID()*/) // если i != ID комнаты, то
		{
			NewRoom->SetRoomID(i);// задаем ей ID
			RoomsVec_.push_back(NewRoom); // добавляем в вектор комнат
			return NewRoom;
		}
	// если не нашлось(все заняты)
	NewRoom->SetRoomID(i+1); // задаем ей ID
	RoomsVec_.push_back(NewRoom); // добавляем в вектор комнат
	return NewRoom;
}
//-----------------------------------------------------------------------
CMGCPServer::ConfParam CMGCPServer::FillinConfParam(MGCP::TMGCP &mgcp, int mode)
{
	ConfParam result; // создаем структуру, которую хотим получить
	auto f = boost::format(string("\n\nv=0\n""o=- %3% 0 IN IP4 %1%\n""s=MGCP_server\n""c=IN IP4 %1%\n"
		"t=0 0\n""a=tool:libavformat 57.3.100\n""m=audio %2% RTP/AVP 8\n"
		"a=rtpmap:8 PCMA/8000\n""a=sendrecv")); // формируем тип ответа

	switch (mode)
	{
	case 1: // случай для предварительного добавления пользователей CRCX
		result.CallID = mgcp.getCallID(); // получаем CALLID для индентификации поинтов
		result.PortsAndAddr.portSrc = GetFreePort(); // улучшить порт-------------------------------
		result.SDPresponse = str(f%EndP_Local().address().to_string() % result.PortsAndAddr.portSrc %(rand()%10000));
		SDPforCRCX_.push_back(result); // добавляем в вектор объявленых
		return result;

	case 2: // случай для дополнения параметров случая 1 в MDCX
		for (auto &entry : SDPforCRCX_)
		{
			//lock lk(mutex_);
			if (entry.CallID == mgcp.getCallID())
			{
				result.CallID = entry.CallID; // получаем CALLID для индентификации поинтов
				result.PortsAndAddr.portSrc = entry.PortsAndAddr.portSrc; // улучшить порт-------------------------------
				result.SDPresponse = str(f%EndP_Local().address().to_string() % result.PortsAndAddr.portSrc % (rand() % 10000));

				result.PortsAndAddr.strAddr = mgcp.cllSDPs[0]->connection.address; // IP адрес из SDP для исходящего RTP
				result.PortsAndAddr.portDest = mgcp.cllSDPs[0]->cllMedia[0].port; //порт из SDP для входящего RTP
				result.PortsAndAddr.ptimeRTP = mgcp.cllSDPs[0]->cllMedia[0].ptime; //из SDP для время вxодящего RTP
				result.PortAsio = GetFreePort(); // порт для srcASio
				result.sdpIn = str(f %result.PortsAndAddr.strAddr % result.PortsAndAddr.portDest % (rand() % 10000)); //вставляем в него порты??
				SDPforCRCX_.erase(std::remove(SDPforCRCX_.begin(), SDPforCRCX_.end(), entry), SDPforCRCX_.end()); // удаляем из вектора объявленных
				return result;
			}
		}
	case 3: // случай полного заполнения CRCX
		result.CallID = mgcp.getCallID(); // получаем CALLID для индентификации поинтов
		result.PortsAndAddr.strAddr = mgcp.cllSDPs[0]->connection.address; // IP адрес из SDP для исходящего RTP
		result.PortsAndAddr.portSrc = GetFreePort(); // улучшить порт-------------------------------
		result.SDPresponse = str(f%EndP_Local().address().to_string() % result.PortsAndAddr.portSrc % (rand() % 10000));
		result.PortsAndAddr.portDest = mgcp.cllSDPs[0]->cllMedia[0].port; //порт из SDP для входящего RTP
		result.PortsAndAddr.ptimeRTP = mgcp.cllSDPs[0]->cllMedia[0].ptime; //из SDP для время вxодящего RTP
		result.PortAsio = GetFreePort(); // порт для srcASio
		result.sdpIn = str(f %result.PortsAndAddr.strAddr % result.PortsAndAddr.portDest % (rand() % 10000)); //вставляем в него порты??
		return result;
	}
	result.error = 1;
	return result;
}
//-----------------------------------------------------------------------
void CMGCPServer::Connectivity(MGCP::TMGCP &mgcp)
{
	using namespace MGCP;
	TEndPoint mgcpEndPnt = { str(boost::format("cnf/%1%") % thsetCnfIdInUse.regnew(1, 1)), mgcp.EndPoint.m_addr };
	auto shpConnect = std::make_shared<CMGCPConnection>(*this, mgcp);
	m_cllConnections[mgcpEndPnt] = shpConnect;
	mgcp.addResponseParam({ TMGCP_Param::SpecificEndPointId, mgcpEndPnt.toString() });
	mgcp.addResponseParam({ TMGCP_Param::ConnectionId, str(boost::format("%1%") % shpConnect->Id()) });
}
//-----------------------------------------------------------------------
void CMGCPServer::proceedCRCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	fprintf(FileLogS, "CRCX\n");
	using namespace MGCP;
	assert(mgcp.CMD == TMGCP::CRCX);
	//asio::io_service io_service;
	string strResponse;
	/*BOOST_SCOPE_EXIT(&strResponse, &mgcp, &udpTO, this_){
		if (strResponse.empty())
			strResponse = mgcp.ResponseOK();
		this_->reply(strResponse, udpTO);
	}BOOST_SCOPE_EXIT_END;*/

	auto conn = findConnection(mgcp);
	if (conn) // вернуть
	{
		const unsigned idConn = thsetAnnIdInUse.regnew(1, 1);
		conn->newSubConnection(idConn, mgcp);
		mgcp.addResponseParam({ TMGCP_Param::ConnectionId, str(boost::format("%1%") % idConn) });
	}
	if (mgcp.EndPoint.m_point == "ann/$")// обработка обычного звонка
	{
		cout << "\nANN-----------------------";
		TEndPoint mgcpEndPnt = {str(boost::format("ann/%1%") % thsetAnnIdInUse.regnew(1, 1)), mgcp.EndPoint.m_addr };
		auto shpConnect = std::make_shared<CMGCPConnection>(*this, mgcp);
		m_cllConnections[mgcpEndPnt] = shpConnect;
		mgcp.addResponseParam({ TMGCP_Param::SpecificEndPointId, mgcpEndPnt.toString() });
		mgcp.addResponseParam({ TMGCP_Param::ConnectionId, str(boost::format("%1%") % shpConnect->Id()) });
		auto f = boost::format(string("\n\nv=0\n""o=- 0 0 IN IP4 %1%\n""s=MGCP_server\n""c=IN IP4 %1%\n"
			"t=0 0\n""a=tool:libavformat 57.3.100\n""m=audio %2% RTP/AVP 8\n""a=rtpmap:8 PCMA/8000\n"
			"a=sendonly"/*//"a=recvonly"//"\nb=AS:64"*/));
		strResponse = mgcp.ResponseOK() + str(f %EndP_Local().address().to_string() % shpConnect->SrcPort());
		reply(strResponse, udpTO);//
		return;
	}
	if (mgcp.EndPoint.m_point == "cnf/$") // обработка первичного запроса конференции
	{
		cout << "\nCNF$-----------------------";
		if (mgcp.parMode != MGCP::TMGCP::ConnMode::confrnce)
		{
			reply(mgcp.ResponseBAD(400) + "cnf/$ - mode! must be confrnce", udpTO);//
			return;
		}
		cout << "\nCNF$-ERROR---------------";
		Connectivity(mgcp);
		cout << "\nCNF$-CONNECT---------------";
		auto room = CreateNewRoom();
		cout << "\nCNF$-finished create room----------";
		assert(room);
		auto confparams = FillinConfParam(mgcp, 3);
		cout << "\nCNF$--FillinConf-----------";
		if (confparams.error == 1)
		{
			reply(mgcp.ResponseBAD(400) + "error in type of filling params", udpTO);
			return;
		}
		room->newInitPoint(confparams.sdpIn, confparams.PortAsio, confparams.PortsAndAddr, confparams.CallID, udpTO.address().to_string());
		cout << "\nCNF$--newInit-------------";
		strResponse = mgcp.ResponseOK() + confparams.SDPresponse;
		reply(strResponse, udpTO);//
		cout << "\nCNF$ ENDENDEND-----------------------";
		return;
	}
	string num = GetRoomIDConn(mgcp.EndPoint.m_point); // получаем номер, куда обращаемся
	string EndpntConf = "cnf/" + num;// создаем вид обращения

	if (mgcp.EndPoint.m_point == EndpntConf) // обработка вторичных запросов конференции
	{
		cout << "\nCNF+++-----------------------";
		if (mgcp.parMode != MGCP::TMGCP::ConnMode::inactive)
		{
			reply(mgcp.ResponseBAD(400) + "cnf/* - mode! must be inactive", udpTO);//
			return;
		}

		//Connectivity(mgcp);
		TEndPoint mgcpEndPnt = { EndpntConf, mgcp.EndPoint.m_addr };
		auto shpConnect = std::make_shared<CMGCPConnection>(*this, mgcp);
		m_cllConnections[mgcpEndPnt] = shpConnect;
		mgcp.addResponseParam({ TMGCP_Param::SpecificEndPointId, mgcpEndPnt.toString() });
		mgcp.addResponseParam({ TMGCP_Param::ConnectionId, str(boost::format("%1%") % shpConnect->Id()) });

		auto confparams = FillinConfParam(mgcp, 1);
		if (confparams.error == 1)
		{
			reply(mgcp.ResponseBAD(400) + "error in type of filling params", udpTO);
			return;
		}
		reply(mgcp.ResponseOK() + confparams.SDPresponse, udpTO);//
		return;
	}
	else
	{
		reply(mgcp.ResponseBAD(400) + "Didnt find mark cnf/*,ann/*", udpTO);//
		return;
	}
}
//-----------------------------------------------------------------------
void CMGCPServer::proceedMDCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	fprintf(FileLogS, "MDCX\n");
	SHP_CMGCPConnection conn = getConnection(mgcp, udpTO);

	if (mgcp.EndPoint.m_point == "ann/$")
	{
		conn->Modify(mgcp);
		reply(mgcp.ResponseOK(), udpTO);
		return;
	}
	string num = GetRoomIDConn(mgcp.EndPoint.m_point); // получаем номер, куда обращаемся
	string EndpntConf = "cnf/" + num;// создаем вид обращения
	
	if (mgcp.EndPoint.m_point == EndpntConf) // обработка вторичных запросов конференции
	{
		if (mgcp.parMode != MGCP::TMGCP::ConnMode::confrnce)
		{
			reply(mgcp.ResponseBAD(400) + "cnf/* - mode! must be confrnce", udpTO);//
			return;
		}
		auto confparams = FillinConfParam(mgcp, 2);
		auto DestRoom = FindRoom(atoi(EndpntConf.c_str()));

		if (!DestRoom) 
		{
			reply(mgcp.ResponseBAD(400) + "cnf/* - couldn`t find room", udpTO);//
			return;
		}
		if (DestRoom->State() == CConfRoom::enmSTATE::stInactive)
		{
			
			DestRoom->newInitPoint(confparams.sdpIn, confparams.PortAsio, confparams.PortsAndAddr, confparams.CallID, udpTO.address().to_string());
			if (DestRoom->GetNumcllPoints() >= 3)
			{
				cout << "\nCONF ROOM START!";
				DestRoom->setState(CConfRoom::stActive);
			}
			conn->Modify(mgcp);
			reply(mgcp.ResponseOK() /*+ confparams.SDPresponse*/, udpTO);//
			return;
		}
		if (DestRoom->State() == CConfRoom::enmSTATE::stActive)
		{
			DestRoom->newDynPoint(confparams.sdpIn, confparams.PortAsio, confparams.PortsAndAddr, confparams.CallID);
			conn->Modify(mgcp);
			reply(mgcp.ResponseOK() /*+ confparams.SDPresponse*/, udpTO);
			return;
		}
		else { reply(mgcp.ResponseBAD(400) + "Wrong state", udpTO); return; }
	}
	else { reply(mgcp.ResponseBAD(400) + "didnt`t find room with this code", udpTO); return; }
}
//-----------------------------------------------------------------------
void CMGCPServer::proceedDLCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	fprintf(FileLogS, "DLCX\n");
	if (mgcp.CMD != MGCP::TMGCP::DLCX)
	{
		reply(mgcp.ResponseBAD(400) + "wrong mgcp command", udpTO); return;
	}
	SHP_CMGCPConnection conn = getConnection(mgcp, udpTO);
	if (conn)
	{

		conn->stopMedia();
		auto CallID = mgcp.getCallID();

		const auto& epMGCP = mgcp.EndPoint;
		unsigned id = 0;
		if (qi::parse(epMGCP.m_point.cbegin(), epMGCP.m_point.cend(),
			"ann/" >> qi::uint_[phoenix::ref(id) = qi::_1]))
		{
			reply(mgcp.ResponseOK(250), udpTO);
			thsetAnnIdInUse.unreg(id);
			return;
		}
		if (!id && qi::parse(epMGCP.m_point.cbegin(), epMGCP.m_point.cend(),
			"cnf/" >> qi::uint_[phoenix::ref(id) = qi::_1]))
		{
			string num = GetRoomIDConn(mgcp.EndPoint.m_point); // получаем номер, куда обращаемся
			string EndpntConf = "cnf/" + num;// создаем вид обращения
			auto DestRoom = FindRoom(atoi(EndpntConf.c_str()));
			if (!DestRoom)
			{
				reply(mgcp.ResponseOK(250), udpTO);
				assert(DestRoom);
			}
			if (DestRoom->GetNumcllPoints() <= 2)
			{
				DeleteRoom(DestRoom);
				reply(mgcp.ResponseOK(250), udpTO);
				thsetCnfIdInUse.unreg(id);
				return;
			}
			if (DestRoom->GetNumcllPoints() > 2)
			{
				SetFreePort(DestRoom, CallID);
				DestRoom->delPoint(CallID);
				reply(mgcp.ResponseOK(250), udpTO);
				thsetCnfIdInUse.unreg(id);
				return;
			}
			else { reply(mgcp.ResponseBAD(400) + "Somethig went wrong with amout of callpoints", udpTO); return; }
		}
		else { reply(mgcp.ResponseBAD(400) + "didn`t find connection code", udpTO); return; }
	}
	else { reply(mgcp.ResponseBAD(400) + "didn`t find connection", udpTO); return; }
}

