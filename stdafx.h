// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#define INPUT_SAMPLERATE     8000
#define INPUT_FORMAT         AV_SAMPLE_FMT_S16
#define INPUT_CHANNEL_LAYOUT AV_CH_LAYOUT_STEREO
#define OUTPUT_BIT_RATE 8000
#define OUTPUT_CHANNELS 1
#define OUTPUT_SAMPLE_FORMAT AV_SAMPLE_FMT_S16
#define VOLUME_VAL 0.90

#define snprintf _snprintf
//#ifdef WIN32
//#include "targetver.h"
//#endif // _WIN32_

#include <stdio.h>
#include <tchar.h>
#ifdef WIN32
#include "targetver.h"
#endif // _WIN32_

//store
#include <string>
#include <vector>
#include <queue>
//#include <map>
//#include <set>

//file
//#include <iostream>
#include <fstream>//
//#include <ostream>

//utils
#include <chrono>
#include <mutex>
#include <thread>
//#include <atomic>
//#include <iomanip>                       
//#include <ctime> 
//#include <exception>


//typedef std::lock_guard<std::mutex> lock;

// TODO: reference additional headers your program requires here
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/asio/basic_datagram_socket.hpp>
#include <boost/thread/thread.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/exception/all.hpp>
//#include <boost/algorithm/string.hpp>
//#include "boost/date_time/gregorian/gregorian.hpp"
//#include <boost/thread.hpp>

typedef boost::error_info<struct tag_errmsg, std::string> errmsg_info;




extern "C"
{
#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/avfilter.h>
#include <libavutil/avstring.h>
#include <libavutil/md5.h>
#include <libavutil/mem.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
	//
#include "libavutil/time.h"
	//#include <libswresample/swresample.h>
	//#include <libavutil/audio_fifo.h>
	//#include <libavformat/avio.h>
	//#include <libavutil/file.h>
	//#include <libavutil/time.h>
	//#include <libavcodec/avcodec.h>
}
//namespace phoenix = boost::phoenix;
//namespace qi = boost::spirit::qi;
//namespace chrono = std::chrono;

namespace asio = boost::asio;
using boost::asio::ip::udp;
using std::string;
using std::cerr;
using std::cout;
using std::shared_ptr;
#pragma comment (lib,"avformat.lib")
#pragma comment (lib,"avcodec.lib")
#pragma comment (lib,"avformat.lib")
#pragma comment (lib,"avutil.lib")
#pragma comment (lib,"avfilter.lib")
#pragma comment (lib,"Winmm.lib")
//Winmm.dll




