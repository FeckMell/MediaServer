// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define snprintf _snprintf
//#ifdef WIN32
//#include "targetver.h"
//#endif // _WIN32_

#include <stdio.h>
#include <tchar.h>
#ifdef WIN32
#include "targetver.h"
#endif // _WIN32_

#include <stdio.h>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <chrono>
#include <mutex>
#include <thread>
#include <atomic>

typedef std::lock_guard<std::mutex> lock;
//#include <tchar.h>

// TODO: reference additional headers your program requires here
#include <boost/asio.hpp>
namespace asio	= boost::asio;
namespace ip	= boost::asio::ip;
using boost::asio::ip::udp;

//#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/scope_exit.hpp>
#include <boost/scoped_ptr.hpp>



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
	//#include <libswresample/swresample.h>
	//#include <libavutil/audio_fifo.h>
	//#include <libavformat/avio.h>
	//#include <libavutil/file.h>
	//#include <libavutil/time.h>
	//#include <libavcodec/avcodec.h>
}

namespace chrono = std::chrono;
using std::string;
using std::cerr;
using std::cout;
using std::shared_ptr;
//FILE *FileLog;




