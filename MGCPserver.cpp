#include "stdafx.h"
#include "MGCPserver.h"
#include "Utils.h"
//#include <string>
//#include <iostream> 
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstddef>
#include <sstream>
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
#include <boost/thread/thread.hpp>
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

static ThreadedSet<unsigned> thsetAnnIdInUse;
static ThreadedSet<unsigned> thsetCnfIdInUse;

/************************************************************************
	CMGCPServer
************************************************************************/
void CMGCPServer::loggit(string a)
{
	fprintf(FileLogServer, ("\n" + a + "\n//-------------------------------------------------------------------").c_str());
	fflush(FileLogServer);
}
CMGCPServer::CMGCPServer(const TArgs& args)
: m_args(args), io_service__(args.io_service1), socket_(args.io_service, udp::endpoint(udp::v4(), args.endpnt.port()))
{
	//fopen_s(&FileLogServer, "LOGS_Server.txt", "w");
	loggit("Server Construct\n");
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
		loggit("void CMGCPServer::do_receive()\n---------" + sender_endpoint_.address().to_string() + " sent:\n" + strTmp);
		printf("--------- %s sent:\n%s\n--------- len %lu ---------\n\n",
			sender_endpoint_.address().to_string().c_str(), strTmp.c_str(), bytes_recvd);
		proceedReceiveBuffer(data_, sender_endpoint_);
	});
}

//-----------------------------------------------------------------------
void CMGCPServer::do_send(std::size_t length)
{

	socket_.async_send_to(
		asio::buffer(data_, length), sender_endpoint_,
		[this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/)
	{
		loggit("void CMGCPServer::do_send");
		do_receive();
	});
}

//-----------------------------------------------------------------------
SHP_CMGCPConnection CMGCPServer::findConnection(const MGCP::TMGCP& mgcp) const
{
	//loggit("SHP_CMGCPConnection CMGCPServer::findConnection");
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
	conn->sendMedia(strFile, CallID);
}

//-----------------------------------------------------------------------
void CMGCPServer::proceedRQNT(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	assert(mgcp.CMD == MGCP::TMGCP::RQNT);
	loggit("void CMGCPServer::proceedRQNT");
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
				loggit("std::thread th(threadSendMedia, conn," + (m_args.strMmediaPath + strFile) + ", " + CallID + ");");
				std::thread th(threadSendMedia, conn, m_args.strMmediaPath + strFile, CallID);
				th.detach();
				//fwrite("RQNT done\n", strlen("RQNT done\n"), 1, file);
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
	const auto start_tp = std::chrono::high_resolution_clock::now();
	//cout << "\n==============recieved=================\n";
	loggit("void CMGCPServer::proceedReceiveBuffer()");
	cout << boost::format("=================== %1% sent:\n%2%\n===================\n") % udpTO % pCh; // верхняя строчка короче
	TMGCP mgcp;
	string sdp = (char*)pCh;
	loggit("Incoming massage was:\n" + sdp);
	std::size_t found = sdp.find("v=0");
	if (found != std::string::npos)
		mgcp.SDP = sdp.substr(found);
	else mgcp.SDP = "NONE";
	if (!parseMGCP(pCh, mgcp))
	{
		reply("Not MGCP", udpTO);
		return;
	}
	
	/*for (int i = 0; i < strlen(pCh); ++i)
	{
		if (pCh[i] != '=') continue;
		sdp = sdp.substr(i - 1, strlen(pCh) - 1);
		break;

	}*/
	const auto end_tp = std::chrono::high_resolution_clock::now();
	const auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end_tp - start_tp);
	loggit("Parsing duration: " + to_string(dur.count()) + "microseconds");
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
	auto f = boost::format("----------REPLIED TO %1%:\n%2%\n------------------\n") % udpTO % str;
	loggit("void CMGCPServer::reply\n" + f.str());
	cout << "\n --------------------Server REPLIED------------------------\n";
	//cout << boost::format("----------REPLIED TO %1%:\n%2%\n------------------\n")% udpTO % str;
}

//-----------------------------------------------------------------------
void CMGCPServer::respond(const string str)
{
	//printf("\nrespond\n");
	socket_.async_send_to(
		asio::buffer(str), sender_endpoint_,
		[this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/)
	{
		loggit("void CMGCPServer::respond");
		do_receive();
	});
}

//-----------------------------------------------------------------------
void CMGCPServer::Run()
{
	loggit("Server.Run()");
	for (;;)
	{
		udp::endpoint sender_endpoint;
		loggit("Waiting for Callagent request...\n");
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
	loggit("bool CMGCPServer::FindPort");
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
		loggit("Found free port =" + to_string(freeport));
		return freeport; // возвращаем порт для использования
	}
	for (unsigned i = 0; i < PortsinUse_.size(); ++i) // поиск по вектору занятых портов
	{
		if (FindPort(freeport)) // если в векторе портов есть такой
			freeport += 2; // тогда смещаемся на 2
		else
		{
			PortsinUse_.push_back(freeport);// иначе добавляем его в занятые порты
			loggit("Found free port =" + to_string(freeport));
			return freeport; // и возвращаем для использования
		}
	}
	PortsinUse_.push_back(freeport); // если дошли до конца вектора, то добавляем в него порт на 2 больше максимального.
	loggit("Found free port =" + to_string(freeport));
	return freeport;// возвращаем для использования
}
//-----------------------------------------------------------------------
/*void CMGCPServer::SetFreePort(SHP_CConfRoom room, string CallID)
{
	PortsinUse_.erase(std::remove(PortsinUse_.begin(), PortsinUse_.end(), room->FindPoint(CallID)->GetPort()), 
		PortsinUse_.end()); // удаляем порт из занятых
}*/
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
	loggit("string CMGCPServer::GetRoomIDConn = " + result);
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
SHP_ConfParam CMGCPServer::FindClient(string CallID)
{
	auto it = std::find_if(SDPforCRCX_.begin(),
		SDPforCRCX_.end(),
		[CallID](const SHP_ConfParam& current) { return current->CallID == CallID; }
	);
	if (it != SDPforCRCX_.end()) return *it;
	else return nullptr;
}
//-----------------------------------------------------------------------
bool CompareRooms(SHP_CConfRoom i, SHP_CConfRoom j){ return (i->GetRoomID()<j->GetRoomID()); }
//-----------------------------------------------------------------------
/*void CMGCPServer::DeleteRoom(SHP_CConfRoom remRoom)
{
	for (auto& point : remRoom->GetIDPoints()) // для всех поинтов освобождаем сокеты
		SetFreePort(remRoom, point);
	RoomsVec_.erase(std::remove(RoomsVec_.begin(), RoomsVec_.end(), remRoom), RoomsVec_.end()); // удаляем комнату
	// надо бы не только из вектора удалить, а вообще. И все поинты
}*/
//-----------------------------------------------------------------------
SHP_CConfRoom CMGCPServer::CreateNewRoom(/*asio::io_service& io_service*/)
{
	SHP_CConfRoom NewRoom(new CConfRoom()); // создаем комнату
	unsigned int i; // предполагаемый номер комнаты
	if (RoomsVec_.size() == 0) //если комнат до этого не было
	{
		NewRoom->SetRoomID(0);// задаем ей ID
		RoomsVec_.push_back(NewRoom); // добавляем в вектор комнат
		loggit("SHP_CConfRoom CMGCPServer::CreateNewRoom()\n Created room with ID=" + to_string(0));
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
			loggit("SHP_CConfRoom CMGCPServer::CreateNewRoom()\n Created room with ID=" + to_string(i));
			return NewRoom;
		}
	// если не нашлось(все заняты)
	NewRoom->SetRoomID(i+1); // задаем ей ID
	RoomsVec_.push_back(NewRoom); // добавляем в вектор комнат
	loggit("SHP_CConfRoom CMGCPServer::CreateNewRoom()\n Created room with ID=" + to_string(i + 1));
	return NewRoom;
}
//-----------------------------------------------------------------------
SHP_ConfParam CMGCPServer::FillinConfParam(MGCP::TMGCP &mgcp, int mode)
{
	SHP_ConfParam result(new ConfParam()); // создаем структуру, которую хотим получить
	result->my_port = GetFreePort();
	result->CallID = mgcp.getCallID();
	//auto f = boost::format(string("\n\nv=0\no=- %3% 0 IN IP4 %1%\ns=%4%\nc=IN IP4 %1%\nt=0 0\na=tool:libavformat 57.3.100\nm=audio %2% RTP/AVP 8\na=rtpmap:8 PCMA/8000\na=ptime:20\n")); // формируем тип ответа
	auto f = boost::format(string("\n\nv=0\no=- %3% 0 IN IP4 %1%\ns=%4%\nc=IN IP4 %1%\nt=0 0\na=tool:libavformat 57.3.100\nm=audio %2% RTP/AVP 97\nb=AS:705\na=rtpmap:97 PCMU/44100/2\n"));// формируем тип ответа
	result->SDPresponse = str(f%EndP_Local().address().to_string() % result->my_port % (rand() % 100000) % (rand() % 100000));
	
	SDPforCRCX_.push_back(result);
	loggit("CMGCPServer::FillinConfParam:result.SDPresponse:\n" + result->SDPresponse + "\nSDP modified From Client:\n" + mgcp.SDP);
	return result;
}
//-----------------------------------------------------------------------
void CMGCPServer::Connectivity(MGCP::TMGCP &mgcp)
{
	using namespace MGCP;
	loggit("void CMGCPServer::Connectivity");
	TEndPoint mgcpEndPnt = { str(boost::format("cnf/%1%") % thsetCnfIdInUse.regnew(1, 1)), mgcp.EndPoint.m_addr };
	auto shpConnect = std::make_shared<CMGCPConnection>(*this, mgcp);
	m_cllConnections[mgcpEndPnt] = shpConnect;
	mgcp.addResponseParam({ TMGCP_Param::SpecificEndPointId, mgcpEndPnt.toString() });
	mgcp.addResponseParam({ TMGCP_Param::ConnectionId, str(boost::format("%1%") % shpConnect->Id()) });
}
//-----------------------------------------------------------------------
void Starttest(SHP_CConfRoom room)
{
	room->Start();
}
void CMGCPServer::proceedCRCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	loggit("void CMGCPServer::proceedCRCX");
	using namespace MGCP;
	assert(mgcp.CMD == TMGCP::CRCX);
	string strResponse;

	auto conn = findConnection(mgcp);
	if (conn) // вернуть
	{
		loggit("if (conn)");
		const unsigned idConn = thsetAnnIdInUse.regnew(1, 1);
		conn->newSubConnection(idConn, mgcp);
		mgcp.addResponseParam({ TMGCP_Param::ConnectionId, str(boost::format("%1%") % idConn) });
	}
	if (mgcp.EndPoint.m_point == "ann/$")// обработка обычного звонка
	{
		//mgcp.cllSDPs
		loggit("if (mgcp.EndPoint.m_point == ann/$)");
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
		loggit("if (mgcp.EndPoint.m_point == cnf/$)");
		if (mgcp.parMode != MGCP::TMGCP::ConnMode::confrnce)
		{
			reply(mgcp.ResponseBAD(400) + "cnf/$ - mode! must be confrnce", udpTO);//
			return;
		}
		Connectivity(mgcp);
		auto room = CreateNewRoom();
		assert(room);
		auto confparams = FillinConfParam(mgcp, 3);
		if (confparams->error == 1)
		{
			reply(mgcp.ResponseBAD(400) + "error in type of filling params", udpTO);
			return;
		}
		loggit("room->NewInitPoint(mgcp.SDP);");

		confparams->input_SDP = DeleteFromSDP(mgcp.SDP, confparams->my_port);
		room->NewInitPoint(confparams->input_SDP, mgcp.SDP);
		//boost::thread my_thread(&Starttest, room);
		//my_thread.detach();
		strResponse = mgcp.ResponseOK() + confparams->SDPresponse;
		reply(strResponse, udpTO);//
		loggit("CRCX: cnf$ END");
		return;
	}
	string num = GetRoomIDConn(mgcp.EndPoint.m_point); // получаем номер, куда обращаемся
	string EndpntConf = "cnf/" + num;// создаем вид обращения

	if (mgcp.EndPoint.m_point == EndpntConf) // обработка вторичных запросов конференции
	{
		loggit("if (mgcp.EndPoint.m_point == " + EndpntConf + ")");
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
		/*if (confparams.error == 1)
		{
			reply(mgcp.ResponseBAD(400) + "error in type of filling params", udpTO);
			return;
		}*/
		reply(mgcp.ResponseOK() + confparams->SDPresponse, udpTO);//
		loggit("CRCX:"+ EndpntConf + " ENDED");
		return;
	}
	else
	{
		reply(mgcp.ResponseBAD(400) + "Didnt find mark cnf/*,ann/*", udpTO);//
		return;
	}
}
//-----------------------------------------------------------------------
void Start(SHP_CConfRoom room)
{
	room->Start();
}
void CMGCPServer::proceedMDCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	loggit("void CMGCPServer::proceedMDCX");
	SHP_CMGCPConnection conn = getConnection(mgcp, udpTO);

	if (mgcp.EndPoint.m_point == "ann/$")
	{
		loggit("it was ann/$");
		conn->Modify(mgcp);
		reply(mgcp.ResponseOK(), udpTO);
		return;
	}
	string num = GetRoomIDConn(mgcp.EndPoint.m_point); // получаем номер, куда обращаемся
	string EndpntConf = "cnf/" + num;// создаем вид обращения
	
	if (mgcp.EndPoint.m_point == EndpntConf) // обработка вторичных запросов конференции
	{
		loggit("if (mgcp.EndPoint.m_point ==" + EndpntConf +")");
		
		if (mgcp.parMode != MGCP::TMGCP::ConnMode::confrnce)
		{
			reply(mgcp.ResponseBAD(400) + "cnf/* - mode! must be confrnce", udpTO);//
			return;
		}
		//auto confparams = FillinConfParam(mgcp, 2);
		auto DestRoom = FindRoom(atoi(EndpntConf.c_str()));
		
		if (!DestRoom) 
		{
			reply(mgcp.ResponseBAD(400) + "cnf/* - couldn`t find room", udpTO);//
			return;
		}
		loggit("found room with ID=" + EndpntConf+ "\nDestRoom->NewInitPoint(mgcp.SDP);");
		//
		auto confparams = FindClient(mgcp.getCallID());
		confparams->input_SDP = DeleteFromSDP(mgcp.SDP, confparams->my_port);
		DestRoom->NewInitPoint(confparams->input_SDP, mgcp.SDP);//SDPresponse);
		if (DestRoom->GetNumCllPoints() == 3)
		{
			loggit("if (DestRoom->GetNumCllPoints() == 3)");
			boost::thread my_thread(&Start,DestRoom);
			my_thread.detach();
			//reply(mgcp.ResponseOK(), udpTO);
		}
		reply(mgcp.ResponseOK(), udpTO);
		loggit("MDCX:" + EndpntConf + "ENDED");
	}
	else { reply(mgcp.ResponseBAD(400) + "didnt`t find room with this code", udpTO); return; }
}
//-----------------------------------------------------------------------
void CMGCPServer::proceedDLCX(MGCP::TMGCP &mgcp, const udp::endpoint& udpTO)
{
	loggit("void CMGCPServer::proceedDLCX");
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
			/*string num = GetRoomIDConn(mgcp.EndPoint.m_point); // получаем номер, куда обращаемся
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
			}*/
			//else { reply(mgcp.ResponseBAD(400) + "Somethig went wrong with amout of callpoints", udpTO); return; }
		}
		else { reply(mgcp.ResponseBAD(400) + "didn`t find connection code", udpTO); return; }
	}
	else { reply(mgcp.ResponseBAD(400) + "didn`t find connection", udpTO); return; }
}

string CMGCPServer::DeleteFromSDP(string inputSDP, int my_port)
{
	string rtp0 = "a=rtpmap:0";
	string rtp18 = "a=rtpmap:18";
	string rtp101 = "a=rtpmap:101";
	string rtpavp = "RTP/AVP";
	string fmtp = "a=fmtp:";
	// delete rtp 18, 101
	std::size_t found = inputSDP.find(rtp18);
	/*if (found != std::string::npos)
		inputSDP.replace(found, inputSDP.find("\n", found + 1) - found + 1, "");
	found = inputSDP.find(rtp101);
	if (found != std::string::npos)
		inputSDP.replace(found, inputSDP.find("\n", found + 1) - found + 1, "");
	found = inputSDP.find(fmtp);
	if (found != std::string::npos)
		inputSDP.replace(found, inputSDP.find("\n", found + 1) - found + 1, "");
	found = inputSDP.find(fmtp);
	if (found != std::string::npos)
		inputSDP.replace(found, inputSDP.find("\n", found + 1) - found + 1, "");
	found = inputSDP.find(fmtp);
	if (found != std::string::npos)
		inputSDP.replace(found, inputSDP.find("\n", found + 1) - found + 1, "");
	// delete rtp 0
	found = inputSDP.find(rtp0);
	if (found != std::string::npos)
		inputSDP.replace(found, inputSDP.find("\n", found + 1) - found + 1, "");*/
	//change IP
	found = inputSDP.find("c=IN IP4");
	if (found != std::string::npos)
		inputSDP.replace(found, inputSDP.find("\n", found + 1) - found + 1, "c=IN IP4 10.77.7.19\n");
	//change port
	found = inputSDP.find("m=audio ");
	if (found != std::string::npos)
		inputSDP.replace(found, inputSDP.find("RTP/AVP", found + 1) - found, "m=audio " + std::to_string(my_port) + " ");
	
	
	//found = inputSDP.find(rtpavp);
	//if (found != std::string::npos)
	//	inputSDP.replace(found, inputSDP.find("\n", found + 1) - found + 1, "RTP/AVP 8\n");
	return inputSDP;

}
