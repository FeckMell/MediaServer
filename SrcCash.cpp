#include "stdafx.h"
#include "SrcCash.h"

std::mutex glb_Mutex;

template <typename _T >
void log(const _T& val)
{
	lock lk(glb_Mutex);
	cout << std::this_thread::get_id() << "\t" << val;
}


/************************************************************************
	CSrcCashEntry
************************************************************************/
SHP_CScopedPFrame CSrcCashEntry::getNextDecoded(bool& bEOF)
{
#if 1
/*
	bEOF = false;
	log(boost::format("%1% prepop (%2%)\n") % Name() % buffFrames_.size());
	SHP_CScopedPFrame frame = buffFrames_.pop();
	log(boost::format("%1% after pop (%2%)\n") % Name() % buffFrames_.size());
	return frame;
*/


	{
/*
		lock lk(pCash_.Mutex());
		frame = buffer_.empty() ? 
			pCash_.pendingData(this, bEOF)
			: buffer_.pop();
*/
	}

	//SHP_CScopedPFrame frame = pCash_.pendingData(this, bEOF); //buffer_.pop(false);

	/*if (buffFrames_.empty())
	{
		pCash_.pendingData(this, bEOF);
		if (!buffFrames_.empty())
			frame = buffFrames_.pop();
	}
	else
		frame = buffFrames_.pop();

*/

	//lock lk(pCash_.Mutex());
	SHP_CScopedPFrame frame;  ;
	if (!buffFrames_.try_pop(frame))
	{
		//log(boost::format("%1% before pop %2%\n") % Name() % buffFrames_.size());
		pCash_.pendingData(this, bEOF);
		buffFrames_.try_pop(frame);

/*
		int i = 0;
		auto f = buffFrames_.pop_noBlocking();
		while(f){
			f = buffFrames_.pop_noBlocking();
			++i;
		}
		cout << i << '\n';*/


	//	log(boost::format("%1% after pop %2%\n") % Name() % buffFrames_.size());

/*
		if (!buffFrames_.empty())
		{
			log(boost::format("%1%\tbefore pop %2%") % Name() % buffFrames_.size());
//			cout << Name() << "\tbefore pop " << buffFrames_.size();
			frame = buffFrames_.pop();
			cout << "\tafter " << buffFrames_.size() << "\t(pending)\n";
			log(boost::format("\tafter %1% \t(pending)\n") % buffFrames_.size());
		}*/
	}
/*
	else
	{
		//log(boost::format("%1% before pop %2% ") % Name() % buffFrames_.size());
			//cout << Name() << "\tbefore pop " << buffFrames_.size();
		frame = buffFrames_.pop();
//		cout << "\tafter " << buffFrames_.size() << '\n';
		log(boost::format("%1% after pop %2%\n") % Name() % buffFrames_.size());

	}*/


	return frame;
#else
	lock lk(pCash_.Mutex());
	SHP_CScopedPFrame shResut;
	bEOF = false;
	//{
		//lock lk(pCash_->Mutex());
		if (buffFrames_.empty())
			pCash_->pendingData(*this, bEOF);
	//}

	if (!buffFrames_.empty())
	{
		shResut = buffFrames_.front();
		if (pts_ > shResut->frame->pkt_pts && shResut->frame->pkt_pts>0)
			assert(false);
		pts_ = shResut->frame->pkt_pts;
		buffFrames_.pop_front();
	}

	return shResut;
#endif
}

/************************************************************************
	CSrcCash
************************************************************************/
SHP_CScopedPFrame CSrcCash::pendingData(const CSrcCashEntry* pEntry, bool& bEOF)
{
	SHP_CScopedPFrame frameNext;

	{
		lock lk(mutex_);
		frameNext = src_->getNextDecoded(bEOF);
	}

	if (frameNext)
	{
		cllEntries_.for_each([&](SHP_CSrcCashEntry entry)
		{
			//if (pEntry != entry.get())
			{
				auto frameCloned = std::make_shared<CScopedPFrame>(frameNext->frame);
				if (entry->buffFrames_.full())
					cerr << boost::format("%1% buffer_.full()\n")
					% entry->Name()
					;
				entry->buffFrames_.push(frameCloned);
			}
		});
	}


	return frameNext;
}

//-----------------------------------------------------------------------
void CSrcCash::run()
{
	bool bEOF = false;
	if (cllEntries_.empty())
		return;

	while (!bEOF)
	{
		SHP_CScopedPFrame frame;
		{
			lock lk(mutex_);
			frame = src_->getNextDecoded(bEOF);
		}

		if (frame)
		{
			cllEntries_.for_each([&](SHP_CSrcCashEntry entry)
			{
				CSrcCashEntry* p = entry.get();
				if (p->buffFrames_.full())
					log(boost::format("%1% entry->buffer_.full()\n") % p->Name());
					//cerr << "entry->buffer_.full()\n";
				p->buffFrames_.push(frame);
				log(boost::format("%1% pushed (%2%)\n") % p->Name() % p->buffFrames_.size());
			});
		}
	}
}

//-----------------------------------------------------------------------
SHP_CSrcCashEntry CSrcCash::newEntry()
{
	SHP_CSrcCashEntry shpResult = std::make_shared<CSrcCashEntry>(*this);
	cllEntries_.reg(shpResult);
	//mapEntries_
	return shpResult;
}

