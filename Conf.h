#pragma once

//#include "SrcCash.h"
#include "DestFusion.h"
#include "SrcAsio.h"
class CSrcCash;
class CDestFusion;
class CConfRoom;
class CSrcCashEntry;
typedef std::shared_ptr<CSrcCashEntry> SHP_CSrcCashEntry;
class ISrcFusion;
typedef std::shared_ptr<ISrcFusion> SHP_ISrcFusion;
struct TRTP_Dest;

/************************************************************************
	CConfPoint
************************************************************************/
class CConfPoint : public boost::noncopyable
{
public:
	//~CConfPoint();

	enum enmSTATE{ stInactive, stSendOnly, stSendRec };
	enmSTATE State() const { return state_; }
	void setState(CConfPoint::enmSTATE val); // Устанавливаем стэйт поинту
	void runDestThread();
	void SetPointID(string CallID){ idPoint = CallID; };// задаем поинту ID
	string GetPointID(){ return idPoint; };// узнаем ID поинта
	int GetPort(){ return socket; };// узнаем порт поинта

private:
	friend class CConfRoom;
	CConfPoint(CConfRoom& rRoom, SHP_ISrcFusion shpSrc, unsigned short port);

	void _addCashedSrc(SHP_CSrcCashEntry shpSrcCashed, string CallID);
	void _removeSrc(SHP_CSrcCashEntry shpSrcCashed, string CallID);
	void _terminateFusion();
	void ShowCashedSrc(SHP_CSrcCashEntry shpSrcCashed);

	std::mutex  mutex_;
	enmSTATE	state_ = stInactive;
	CConfRoom&	refRoom_;
//	SHP_ISrcFusion shpSrc_;
	const boost::scoped_ptr<CSrcCash> scpCash_;
	const boost::scoped_ptr<CDestFusion> scpFusion_;
	boost::scoped_ptr<std::thread> scpThreadFusion_;
	string idPoint; // CALLID поинта
	int socket; // приписанный этому поинту порт
};

typedef std::shared_ptr<CConfPoint> SHP_CConfPoint;


/************************************************************************
	CConfRoom
************************************************************************/
class CConfRoom : public boost::noncopyable
{
public:
	CConfRoom(asio::io_service& io_service);
	enum enmSTATE{ stInactive, stActive};

	enmSTATE State() /*const*/; //{ printf("\n stste="); cout << state_; return state_; }
	//bool State2(bool a);// { return false; }; for test
	void setState(CConfRoom::enmSTATE val);// устанавливает  State
	unsigned GetNumcllPoints(){ return cllPoints_.size(); }// узнаем сколько всего поинтов
	void SetRoomID(int ID){ RoomID = ID; }// устанавливаем комнате ID
	int GetRoomID(){ return RoomID; }// узнаем ID комнаты
	std::vector<string> GetIDPoints();// получаем вектор ID всех поинтов для удаления комнаты
	SHP_CConfPoint FindPoint(string CallID);// находим поинт по ID

	void delPoint(string CallID/*SHP_CConfPoint shpPnt/*,  unsigned short port, const TRTP_Dest& rtpdest*/); // удаление поинта
	void newInitPoint(const string& sdp_string, unsigned short port, const TRTP_Dest& rtpdest, string CallID, string IP);// добавления поинта при stinactive
	void newDynPoint(const string& sdp_string, unsigned short port, const TRTP_Dest& rtpdest, string CallID);// добавление поинта при stactive
//	void newDynPoint(CMGCPServer::ConfParam params);
//	void newInitPoint(CMGCPServer::ConfParam params);
	//void newInitPoint(const string& strFile,  const TRTP_Dest& rtpdest);
private:
	void _remove(SHP_CConfPoint, string CallID);
	void _add(SHP_CConfPoint, string CallID);

	std::mutex  mutex_;
	enmSTATE state_ = stInactive; //TODO: Atomic?
	const boost::scoped_ptr<CSrcCash> scpSrcCash_;
	std::vector<SHP_CConfPoint> cllPoints_;
	asio::io_service &io_service_;
	int RoomID;
};
typedef std::shared_ptr<CConfRoom> SHP_CConfRoom;