#pragma once
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

/* <\> FFMPEG PARAMETERS */
#define INPUT_SAMPLERATE     8000
#define INPUT_FORMAT         AV_SAMPLE_FMT_S16
#define INPUT_CHANNEL_LAYOUT AV_CH_LAYOUT_MONO
#define OUTPUT_BIT_RATE 8000
#define OUTPUT_CHANNELS 1
#define OUTPUT_SAMPLE_FORMAT AV_SAMPLE_FMT_S16
#define VOLUME_VAL 1.00 
/* </> FFMPEG PARAMETERS */


#ifdef WIN32
#include <SDKDDKVer.h>
//#include <tchar.h>//
#endif


/* <\> C++ */
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#ifndef WIN32
#include <fstream>
#endif
#include <chrono>
#include <thread>
#include <map>
#include <regex>
/* </> C++ */


/* <\> BOOST*/
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/signals2.hpp>
//#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/circular_buffer.hpp>

#undef BOOST_ASIO_ERROR_CATEGORY_NOEXCEPT 
/* </> BOOST */


/* <\>FFMPEG */
extern "C"
{
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/avfilter.h>
#include <libavutil/opt.h> //#include <libavutil/avstring.h>//#include <libavutil/md5.h>//#include <libavutil/mem.h>//#include <libavutil/samplefmt.h>//#include "libavutil/time.h"
}

#ifdef WIN32
#pragma comment (lib,"avformat.lib")
#pragma comment (lib,"avcodec.lib")
#pragma comment (lib,"avformat.lib")
#pragma comment (lib,"avutil.lib")
#pragma comment (lib,"avfilter.lib")

#endif
/* </> FFMPEG */

#ifdef WIN32
#include "Mmsystem.h"
#pragma comment (lib,"Winmm.lib")
#endif//TimeBeginPeriod()

/* <\> GLOBAL TYPEDEFs */
typedef boost::asio::io_service IO; // shortcut for io_service
typedef boost::asio::ip::udp::endpoint EP; // shortcut for endpoint

typedef std::shared_ptr<boost::asio::io_service> SHP_IO;
typedef std::shared_ptr<std::thread> SHP_thread;
/* </> GLOBAL TYPEDEFs */

using namespace std;
