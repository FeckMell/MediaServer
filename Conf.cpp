#include "stdafx.h"
#include "Conf.h"
#include <WinBase.h>
#include <Windows.h>

/************************************************************************
	CConfRoom
************************************************************************/
CConfRoom::CConfRoom(asio::io_service& io_service) : /*scpSrcCash_(new CSrcCash),*/ io_service_(io_service) //вернуть с первым коментом*/
{
	//asio::io_service io_service_;
	//io_service_ = io_service;
	//io_service_.run();
}


//-----------------------------------------------------------------------
void CConfRoom::_add(SHP_CConfPoint shpNewPnt, string CallID)
{
	assert(shpNewPnt);
	{
		lock lk(mutex_);
		for (auto& pnt : cllPoints_)
		{
			//Из всех существующих взять источник
			cout << "\n addCashedSrc";
			shpNewPnt->_addCashedSrc(pnt->scpCash_->newEntry(CallID), pnt->idPoint);
			//Всем существующим добавить новый источник
			pnt->_addCashedSrc(shpNewPnt->scpCash_->newEntry(pnt->idPoint), CallID);
		}
		
	}
	cllPoints_.push_back(shpNewPnt);
}

//-----------------------------------------------------------------------
void CConfRoom::_remove(SHP_CConfPoint shpRemPnt, string CallID)
{
	assert(shpRemPnt);
	{
		lock lk(mutex_);
		// из callpoints_ удаляем запись об этом клиенте
		cllPoints_.erase(std::remove(cllPoints_.begin(), cllPoints_.end(), shpRemPnt), cllPoints_.end());
		for (auto& pnt : cllPoints_)
		{
			//Всем существующим удалить источник
			pnt->_removeSrc(pnt->scpCash_->removeEntry(CallID), pnt->idPoint);
		}
	}
}
//-----------------------------------------------------------------------
std::vector<string> CConfRoom::GetIDPoints()
{
	std::vector<string> resultVec;
	for (auto& point : cllPoints_)
		resultVec.push_back(point->idPoint);
	return resultVec;
}
//-----------------------------------------------------------------------
SHP_CConfPoint CConfRoom::FindPoint(string CallID)
{
	auto it = std::find_if(cllPoints_.begin(),
		cllPoints_.end(),
		[CallID](const SHP_CConfPoint& current) { return current->idPoint == CallID; }
	);

	if (it != cllPoints_.end()) return *it;
	else return nullptr;
}
//-----------------------------------------------------------------------
void CConfRoom::delPoint(string CallID)
{
	auto shpPnt = FindPoint(CallID);
	_remove(shpPnt, CallID);
	//shpPnt->scpFusion_->terminate();
	shpPnt->_terminateFusion();
	//TerminateThread(shpPnt->scpThreadFusion_->native_handle(),0 );
	//cout<<"\nshpPnt->_terminateFusion(); done";
	/*
	for (auto entry : cllPoints_)
	{
		printf("\nentry->runDestThread(); start\n");
		entry->runDestThread();
		
	}*/
	//cou << "\nend of delpoint";
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
void CConfRoom::newInitPoint(const string& sdp_string, unsigned short port,
	const TRTP_Dest& rtpdest, string CallID, string IP)
{
	cout << "\nCreate CsrcAsio";
	cout << " io_service="<<&io_service_;
	SHP_ISrcFusion shpSrc(new CSrcAsio(sdp_string, io_service_, rtpdest.portSrc/*port*/,IP));//1
	cout << "\nCreate CConfPoint";
	SHP_CConfPoint shpPnt(new CConfPoint(*this, shpSrc, rtpdest.portSrc/*port*/));//1
	cout << "SetPointID";
	shpPnt->SetPointID(CallID);
	cout << "\n_add";
	_add(shpPnt, CallID);
	cout << "\nopenRTP";
	shpPnt->scpFusion_->openRTP(/*rtpdest*/{rtpdest.strAddr, rtpdest.portDest, port, rtpdest.ptimeRTP});//1
}
/*void  CConfRoom::newInitPoint(CMGCPServer::ConfParam params)
{
	SHP_ISrcFusion shpSrc(new CSrcAsio(params.sdpIn, io_service_, params.PortAsio));
	SHP_CConfPoint shpPnt(new CConfPoint(*this, shpSrc, params.PortAsio));
	shpPnt->scpFusion_->openRTP(params.PortsAndAddr);
	shpPnt->SetPointID(params.CallID);
	_add(shpPnt, params.CallID);
}*/
//newDynPoint(const string& sdp_file...
/*void CConfRoom::newDynPoint(const string& sdp_file, unsigned short port,
	const TRTP_Dest& rtpdest)
{
	SHP_ISrcFusion shpSrc(new CSrcAsio(sdp_file, io_service_, port));
	SHP_CConfPoint shpPnt(new CConfPoint(*this, shpSrc));

	shpPnt->scpFusion_->openRTP(rtpdest);

	_add(shpPnt);
	shpPnt->runDestThread();
}*/
void CConfRoom::newDynPoint(const string& sdp_string, unsigned short port,
	const TRTP_Dest& rtpdest, string CallID)
{
	SHP_ISrcFusion shpSrc(new CSrcAsio(sdp_string, io_service_, port, "111.111.111.111"));
	SHP_CConfPoint shpPnt(new CConfPoint(*this, shpSrc, port));
	shpPnt->scpFusion_->openRTP(rtpdest);
	shpPnt->SetPointID(CallID);
	_add(shpPnt, CallID);
	shpPnt->runDestThread();
}
/*void CConfRoom::newDynPoint(CMGCPServer::ConfParam params)
{
	SHP_ISrcFusion shpSrc(new CSrcAsio(params.sdpIn, io_service_, params.PortAsio));
	SHP_CConfPoint shpPnt(new CConfPoint(*this, shpSrc, params.PortAsio));
	shpPnt->scpFusion_->openRTP(params.PortsAndAddr);
	shpPnt->SetPointID(params.CallID);
	_add(shpPnt, params.CallID);
	//shpPnt->SayStopThread(false);
	shpPnt->runDestThread();
}*/
/*
//-----------------------------------------------------------------------
 void CConfRoom::newInitPoint(const string& strFile, const TRTP_Dest& rtpdest)
{
	SHP_ISrcFusion shpSrc(new CSrcCommon(strFile));
	SHP_CConfPoint shpPnt(new CConfPoint(*this, shpSrc));

	shpPnt->scpFusion_->openRTP(rtpdest);

	_add(shpPnt);
}
*/
//-----------------------------------------------------------------------
void CConfRoom::setState(enmSTATE val)
{
	cout << "\n SetStateRoom";
	if (state_ == val)
		return;
	state_ = val;
	if (val == stActive)
	{
		//cout << "\n setstate: state active";
		for (auto& point : cllPoints_)
		{
			cout << "\n 1 RundestThread";
			point->runDestThread();
		}		
	}
}
//-----------------------------------------------------------------------
CConfRoom::enmSTATE CConfRoom::State()
{
	return state_;
}
/************************************************************************
	CConfPoint	                                                                     
************************************************************************/
CConfPoint::CConfPoint(CConfRoom& rRoom, SHP_ISrcFusion shpSrc, unsigned short port)
: scpFusion_(new CDestFusion), refRoom_(rRoom), scpCash_(new CSrcCash(shpSrc))
//shpSrc_(shpSrc)
{
	socket = port;
}
//-----------------------------------------------------------------------
void CConfPoint::setState(enmSTATE val)
{
	if (state_ == val)
		return;
	state_ = val;

	if (val != stInactive)
	{
		runDestThread();
		//state_ = val;
	}
}
void CConfPoint::ShowCashedSrc(SHP_CSrcCashEntry shpSrcCashed){}
//-----------------------------------------------------------------------
void CConfPoint::_addCashedSrc(SHP_CSrcCashEntry shpSrcCashed, string CallID)
{
	cout << "\n  addSrcRef";
	scpFusion_->addSrcRef(shpSrcCashed, CallID);
	cout << "\n  addSrcRef end";
}

//-----------------------------------------------------------------------
void CConfPoint::_removeSrc(SHP_CSrcCashEntry shpSrcCashed, string CallID)
{
	scpFusion_->terminate();
	scpFusion_->remSrcRef(shpSrcCashed, CallID);
}

//-----------------------------------------------------------------------
void CConfPoint::_terminateFusion()
{
	if (scpThreadFusion_)
	{
		scpThreadFusion_->join();
		scpThreadFusion_.reset();
	}
}

//-----------------------------------------------------------------------
void CConfPoint::runDestThread()
{
	cout << "\n  1 _terminateFusion();";
	_terminateFusion();
	cout << "\n  1 &CDestFusion::run";
	scpThreadFusion_.reset( new std::thread(&CDestFusion::run, std::ref(scpFusion_))  );
}



