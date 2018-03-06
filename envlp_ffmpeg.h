#pragma once

extern "C"
{
//#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
//#include <libavformat/avio.h>
//#include <libavutil/file.h>
//#include <libavutil/time.h>
#include <libavutil/opt.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/avfilter.h>
//#include <libswresample/swresample.h>
//#include <libavutil/audio_fifo.h>
#include <libavutil/avstring.h>
}


extern const AVFilter* gl_abuffer;
extern const AVFilter* gl_abuffersink;
extern const AVFilter* gl_amix;
string get_error_text(const int error);

struct CAVPacket : AVPacket
{
	CAVPacket() : AVPacket()
	{
		av_init_packet(this);
		data = nullptr;
		size = 0;
	}
	CAVPacket(size_t sz) : CAVPacket()
	{
		if (sz > 0)
			av_new_packet(this, sz);
	}
	int grow_by(int by)
	{
		return av_grow_packet(this, by);
	}
	void shrink_to(int to)
	{
		av_shrink_packet(this, to);
	}
	~CAVPacket(){ av_free_packet(this); }

	operator bool()const{ return data != nullptr; }
	//void free(){ av_free_packet(this); }
};

typedef shared_ptr<CAVPacket> SHP_CAVPacket;
typedef std::function<void(SHP_CAVPacket)> FN_SHPPacket;

class CScopedPFrame : boost::noncopyable
{
public:
	CScopedPFrame() {
		/*
			#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
			#define av_frame_alloc  avcodec_alloc_frame
			#endif
		*/
		frame = av_frame_alloc();
		if (!frame)
			std::cerr << "Can't alloc av_frame_alloc in " << __LINE__;
	}
	CScopedPFrame(const AVFrame *frmsrc) {
		frame = av_frame_clone(frmsrc);
	}

	~CScopedPFrame(){
		av_frame_free(&frame);
	}

	bool isValid()const{ return frame != nullptr; }
	operator bool()const{ return isValid(); }
	operator AVFrame *()const{ return frame; }
	AVFrame * operator ->()const{ return frame; }

	AVFrame *frame;
};
typedef shared_ptr<CScopedPFrame> SHP_CScopedPFrame;
typedef std::function<void(SHP_CScopedPFrame)> FN_SHPFrame;

class CAVFilterGraph : boost::noncopyable
{
public:
	CAVFilterGraph(){
		pGraph_ = avfilter_graph_alloc();
	}
	~CAVFilterGraph(){
		avfilter_graph_free(&pGraph_);
	}
	bool isValid()const{ return pGraph_ != nullptr; }
	void DestroyGraph(){ avfilter_graph_free(&pGraph_);}//
	AVFilterGraph* operator ->()const{ return pGraph_; }
	operator AVFilterGraph*()const{ return pGraph_; }
private:
	AVFilterGraph* pGraph_ = nullptr;
};

//TOD0: make throw ffmpeg exceptions

#define RETURN_ON_AVERROR(func, msg) do{m_lastError = func; \
if (m_lastError < 0){\
	std::cerr << "Error '" << msg \
	<< boost::format("' in %1%(%2%). FFMPEG: %3% (code %4%).\n") \
	% __FILE__ % __LINE__ \
	% get_error_text(m_lastError) % m_lastError; \
	return; \
} \
} while (false)

#define RETURN_AVERROR(func, msg) do{ m_lastError = func; \
if (m_lastError < 0){ \
	std::cerr << "Error '" << msg \
	<< boost::format("' in %1%(%2%). FFMPEG: %3% (code %4%).\n") \
	% __FILE__ % __LINE__ \
	% get_error_text(m_lastError) % m_lastError; \
	return m_lastError; \
} \
} while (false) 

#define RETERR_ON_FALSE(state, func, msg) do{ \
if (!(state)) \
RETURN_AVERROR(func, msg); \
} while (false) 
