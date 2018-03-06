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
	void setState(CConfPoint::enmSTATE val); // ������������� ����� ������
	void runDestThread();
	void SetPointID(string CallID){ idPoint = CallID; };// ������ ������ ID
	string GetPointID(){ return idPoint; };// ������ ID ������
	int GetPort(){ return socket; };// ������ ���� ������

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
	string idPoint; // CALLID ������
	int socket; // ����������� ����� ������ ����
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
	void setState(CConfRoom::enmSTATE val);// �������������  State
	unsigned GetNumcllPoints(){ return cllPoints_.size(); }// ������ ������� ����� �������
	void SetRoomID(int ID){ RoomID = ID; }// ������������� ������� ID
	int GetRoomID(){ return RoomID; }// ������ ID �������
	std::vector<string> GetIDPoints();// �������� ������ ID ���� ������� ��� �������� �������
	SHP_CConfPoint FindPoint(string CallID);// ������� ����� �� ID

	void delPoint(string CallID/*SHP_CConfPoint shpPnt/*,  unsigned short port, const TRTP_Dest& rtpdest*/); // �������� ������
	void newInitPoint(const string& sdp_string, unsigned short port, const TRTP_Dest& rtpdest, string CallID, string IP);// ���������� ������ ��� stinactive
	void newDynPoint(const string& sdp_string, unsigned short port, const TRTP_Dest& rtpdest, string CallID);// ���������� ������ ��� stactive
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