#pragma once

//#include "envlp_ffmpeg.h"
//#include "Utils.h"
//#include "ISrcFusion.h"
#include "SrcCash.h"

class ISrcFusion;
typedef std::shared_ptr<ISrcFusion> SHP_ISrcFusion;

/************************************************************************
	TRTP_Dest
************************************************************************/
struct TRTP_Dest
{
	string	 strAddr;
	uint16_t portDest;
	uint16_t portSrc;
	uint16_t ptimeRTP;
};


/************************************************************************
	CDestFusion	                                                                     
************************************************************************/
class CDestFusion : public boost::noncopyable
{
public:
	CDestFusion() { ; }
	CDestFusion(const TRTP_Dest& dest)
	{
		cout << "\nOPEN RTP(dest)\n";
		openRTP(dest);
	}
	CDestFusion(const char *filename) {
		openFile(filename);		
	}
	~CDestFusion(){ _cleanup(); }
	
	int run();
	int runBegin();
	int proceedIO_step();
	int runEnd();

	int openRTP(const TRTP_Dest&);
	int openFile(const char *filename);

	int addSrcRef(SHP_ISrcFusion shpSrc, string CallID);
	int remSrcRef(SHP_ISrcFusion shpSrc, string CallID);

	void terminate(){ buffRTP_.terminate(); }

	//Const methods
	bool isActive()const{ return !buffRTP_.isTerminated(); }
	bool ExistsNotFinishedSrc() const;
	bool isValid() const{ return m_lastError >= 0; }
	int LastError() const { return m_lastError; }
	void SayStopThread(bool val){ nonstop = val; }
	bool nonstop;

private:
	typedef chrono::time_point<chrono::high_resolution_clock> time_point;

	struct TSrcRef
	{
		bool operator ==(const TSrcRef &rhv) const //перегрузка оператора присваивания и здесь же ошибка 
		{
			return CallID == rhv.CallID;
		}

		const char* name()const{
			assert(pctxFilter);
			return pctxFilter->name;
		}
		string CallID;
		SHP_ISrcFusion shpSrc;
		AVFilterContext* pctxFilter = nullptr;
		bool finished		= false;
		bool to_read		= true;
		size_t total_samples= 0;
	};

	AVCodecContext* CodecCTX() const{
		assert(ctxFormat_->nb_streams > 0);
		return ctxFormat_->streams[0]->codec;
	}
	int  _writeRTPpacket(time_point& tp, CAVPacket& pktRTP);
	int  _encode_audio_frame(AVFrame *frame, int *data_present);
	int  _initGraph();
	int  _initMixFilter();
	int  _initSink();
	int  _pullAndEncode();
	int  _proceedIO();

	void _reinitFilters();
	void _dumpGraph();
	void _cleanup();
	void _threadRTPfunction();
	void _finalizeRTPThread();

	int					m_lastError = 0;
	bool				isRTP_		= false;
	unsigned short		ptimeRTP_	= 0;
	AVFormatContext*	ctxFormat_	= nullptr;
	AVFilterContext*	ctxSink_	= nullptr;
	AVFilterContext*	ctxMix_		= nullptr;
	CAVFilterGraph		filtGraf_;
	std::thread*		pRTPThread_ = nullptr;
	std::vector<TSrcRef> cllSrcRefs_;
	CThreadedCircular<SHP_CAVPacket> buffRTP_{ 5 };
};



