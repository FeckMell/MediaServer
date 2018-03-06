#include "stdafx.h"
#include "Conf.h"
#include "SrcCash.h"
#include "DestFusion.h"
#include "SrcAsio.h"

/************************************************************************
	CConfRoom
************************************************************************/
CConfRoom::CConfRoom(asio::io_service& io_service)
: /*scpSrcCash_(new CSrcCash), */io_service_(io_service)
{
	
}

//-----------------------------------------------------------------------
void CConfRoom::_add(SHP_CConfPoint shpNewPnt)
{
	assert(shpNewPnt);

	lock lk(mutex_);

	for (auto& pnt : cllPoints_)
	{
		//¬сем существующим добавить новый источник
		pnt->_addCashedSrc(shpNewPnt->scpCash_->newEntry());

		//»з всех существующих вз€ть источник
		shpNewPnt->_addCashedSrc(pnt->scpCash_->newEntry());
	}

	cllPoints_.push_back(shpNewPnt);
}


//-----------------------------------------------------------------------
void CConfRoom::newPoint(const string& sdp_file, unsigned short port, 
	const TRTP_Dest& rtpdest)
{
	SHP_ISrcFusion shpSrc (new CSrcAsio(sdp_file, io_service_, port));
	SHP_CConfPoint shpPnt (new CConfPoint(*this, shpSrc));
	
	shpPnt->scpFusion_->openRTP(rtpdest);

	_add(shpPnt);
}

//-----------------------------------------------------------------------
void CConfRoom::newPoint(const string& strFile, const TRTP_Dest& rtpdest)
{
	SHP_ISrcFusion shpSrc(new CSrcCommon(strFile));
	SHP_CConfPoint shpPnt(new CConfPoint(*this, shpSrc));

	shpPnt->scpFusion_->openRTP(rtpdest);

	_add(shpPnt);
}

//-----------------------------------------------------------------------
void CConfRoom::setState(enmSTATE val)
{
	if (state_ == val)
		return;

	state_ = val;
	if (val == stActive)
	{
		lock lk(mutex_);
		for (auto& point : cllPoints_)
			point->runDestThread();
	}
}

/************************************************************************
	CConfPoint	                                                                     
************************************************************************/
CConfPoint::CConfPoint(CConfRoom& rRoom, SHP_ISrcFusion shpSrc)
: scpFusion_(new CDestFusion), refRoom_(rRoom), scpCash_(new CSrcCash(shpSrc))
//shpSrc_(shpSrc)
{
	
}

CConfPoint::~CConfPoint()
{
	_terminateFusion();
}

//-----------------------------------------------------------------------
void CConfPoint::setState(enmSTATE val)
{
	if (state_ == val)
		return;

	runDestThread();


	state_ = val;
}

//-----------------------------------------------------------------------
void CConfPoint::_addCashedSrc(SHP_CSrcCashEntry shpSrcCashed)
{
	scpFusion_->addSrcRef(shpSrcCashed);
}

//-----------------------------------------------------------------------
void CConfPoint::_removeSrc(SHP_CSrcCashEntry shpSrcCashed)
{

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
	_terminateFusion();
	scpThreadFusion_.reset(
		new std::thread(&CDestFusion::run, std::ref(scpFusion_))
		);
}


