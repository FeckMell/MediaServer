#pragma once
#include "ISrcFusion.h"

/************************************************************************
	CSrcCommon	                                                                     
************************************************************************/
class CSrcCommon : public ISrcFusion, boost::noncopyable
{
public:
	CSrcCommon(){}
	CSrcCommon(const string& filename)
	{
		open(filename);
	}
	virtual ~CSrcCommon(){
		_closeFormat();
	}
	int open(const string& filename);
	SHP_CScopedPFrame getNextDecoded(bool& bEOF) override;
	const AVCodecContext *CodecCTX()const override{ return _CodecCTX(); };

	const string& Name() const override { return strName_; }


protected:
	AVFormatContext * _CtxFormat() const { return ctxFormat_; }
	AVCodecContext *_CodecCTX()const
	{
		assert(ctxFormat_ != nullptr && idxStream_ >= 0);
		return ctxFormat_->streams[idxStream_]->codec;
	}	
	void _closeFormat();
	int m_lastError = 0;

private:
	int decode_audio_frame(AVFrame *frame, int *data_present, bool *finished);

	int idxStream_ = -1;
	AVFormatContext *ctxFormat_ = nullptr;

	string strName_;

};
