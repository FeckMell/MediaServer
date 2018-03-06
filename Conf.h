#pragma once

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
	~CConfPoint();

	enum enmSTATE{ stInactive, stSendOnly, stSendRec };
	enmSTATE State() const { return state_; }
	void setState(CConfPoint::enmSTATE val);
	void runDestThread();

private:
	friend class CConfRoom;
	CConfPoint(CConfRoom& rRoom, SHP_ISrcFusion shpSrc);

	void _addCashedSrc(SHP_CSrcCashEntry shpSrcCashed);
	void _removeSrc(SHP_CSrcCashEntry shpSrcCashed);
	void _terminateFusion();

	std::mutex  mutex_;
	enmSTATE	state_ = stInactive;
	CConfRoom&	refRoom_;
//	SHP_ISrcFusion shpSrc_;
	const boost::scoped_ptr<CSrcCash> scpCash_;
	const boost::scoped_ptr<CDestFusion> scpFusion_;
	boost::scoped_ptr<std::thread> scpThreadFusion_;
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

	enmSTATE State() const {  return state_; }
	void setState(CConfRoom::enmSTATE val);
	void delPoint(SHP_CConfPoint shpPnt,  unsigned short port, const TRTP_Dest& rtpdest);
	void newPoint(const string& sdp_file, unsigned short port, const TRTP_Dest& rtpdest);
	void newPoint(const string& strFile,  const TRTP_Dest& rtpdest);
private:
	void _remove(SHP_CConfPoint);
	void _add(SHP_CConfPoint);

	std::mutex  mutex_;
	enmSTATE state_ = stInactive; //TODO: Atomic?
//	const boost::scoped_ptr<CSrcCash> scpSrcCash_;
	std::vector<SHP_CConfPoint> cllPoints_;
	asio::io_service& io_service_;

};