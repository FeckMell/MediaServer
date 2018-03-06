#pragma once
#include "Utils.h"
#include "ISrcFusion.h"
class CSrcCashEntry;
typedef std::shared_ptr<CSrcCashEntry> SHP_CSrcCashEntry;

/************************************************************************
CSrcCash
************************************************************************/
class CSrcCash : boost::noncopyable
{
public:
	CSrcCash(SHP_ISrcFusion src) : src_(src){}
	struct TEnty
	{
		boost::circular_buffer<SHP_CScopedPFrame>	frames;
	};
	SHP_CSrcCashEntry newEntry();
	SHP_CScopedPFrame pendingData(const CSrcCashEntry*, bool& bEOF);
	//std::mutex& Mutex() const { return mutex_; }
	const AVCodecContext *CodecCTX()const { return src_->CodecCTX(); };
	const string& Name() const { return src_->Name(); };
	void run();
private:

	SHP_ISrcFusion src_;
	ThreadedSet<SHP_CSrcCashEntry> cllEntries_;
	//	std::map <SHP_ISrcFusion, boost::circular_buffer<SHP_CScopedPFrame> > mapEntries_;
	mutable std::mutex			mutex_;
};

/************************************************************************
	CSrcCashEntry                                                                  
************************************************************************/
class CSrcCashEntry : public ISrcFusion, boost::noncopyable
{
public:
	CSrcCashEntry(CSrcCash&	pCash)
		: pCash_(pCash){
		strName_ = str(boost::format("%1%_%2%") % this % pCash_.Name());
	}

	SHP_CScopedPFrame getNextDecoded(bool& bEOF) override;
	const AVCodecContext *CodecCTX()const override { return pCash_.CodecCTX(); };
	virtual const string& Name()const  override { return strName_; };
private:
	friend class CSrcCash;

	CSrcCash&				pCash_;
//	const SHP_ISrcFusion	src_;
	//boost::circular_buffer<SHP_CScopedPFrame>	buffer_{ 150 };
	CThreadedCircular<SHP_CScopedPFrame> buffFrames_{ 30};
	string strName_;
};


/*
template<typename class _Kty, class _Ty>class ThreadedMap
{
public:
	void reg(const _T& key)
	{
		lock lk(mutex_);
		cll_.insert(key);
	}

	void unreg(const _T& key)
	{
		lock lk(mutex_);
		cll_.erase(key);
	}
	void clear()
	{
		lock lk(mutex_);
		cll_.clear();
	}
	void for_each(std::function<void(const _Ty&)> fn)
	{
		lock lk(mutex_);
		for (const _T& entry : cll_)
		{
			fn(entry);
		}
	}
	size_t size()const{
		lock lk(mutex_);
		return cll_.size();
	}

private:
	std::map<_Kty, class _Ty>		cll_;
	mutable std::mutex	mutex_;
};
*/


