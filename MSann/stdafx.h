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

#undef BOOST_ASIO_ERROR_CATEGORY_NOEXCEPT 
#ifdef WIN32
#define snprintf _snprintf
#include <tchar.h>
#include "targetver.h"
#endif
#include <stdio.h>


//store
#include <string>
#include <vector>
#include <queue>
//#include <map>
//#include <set>

//file
#include <iostream>
#include <fstream>//
#include <ostream>

//utils
#include <chrono>
#include <mutex>
#include <thread>
#include <future>         // std::async, std::future
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
/*LOGS*/
#include <boost/log/core.hpp>//
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>//
#include <boost/log/sinks/text_file_backend.hpp>//
#include <boost/log/utility/setup/file.hpp>//
#include <boost/log/utility/setup/common_attributes.hpp>//
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
using namespace logging::trivial;
/*LOGS END*/

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
}


//namespace asio = boost::asio;
//using boost::asio::ip::udp;
//using std::string;
//using std::cerr;
//using std::cout;
//using std::shared_ptr;
#ifdef WIN32
#pragma comment (lib,"avformat.lib")
#pragma comment (lib,"avcodec.lib")
#pragma comment (lib,"avformat.lib")
#pragma comment (lib,"avutil.lib")
#pragma comment (lib,"avfilter.lib")
#pragma comment (lib,"Winmm.lib")
#endif
#ifdef WIN32
#include "Mmsystem.h"
#endif
//Winmm.dll
typedef std::lock_guard<std::mutex> lock;
typedef boost::asio::io_service IO;
typedef boost::asio::ip::udp::endpoint EP;
typedef std::shared_ptr<IO> SHP_IO;


// TODO: reference additional headers your program requires here
