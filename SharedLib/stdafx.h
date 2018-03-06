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
#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define snprintf _snprintf //use boost format or something else
#include "targetver.h"
#include <tchar.h>
#include "targetver.h"
#endif


/* <\> C++ */
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <map>
#include <regex>
/* </> C++ */


/* <\> BOOST*/
#include <boost/asio.hpp>
#include <boost/format.hpp>
//#include <boost/asio/basic_datagram_socket.hpp>
#include <boost/thread/thread.hpp>
#include <boost/filesystem/operations.hpp>
//#include <boost/exception/all.hpp>

#undef BOOST_ASIO_ERROR_CATEGORY_NOEXCEPT 
/* <\> BOOST LOGS */
#include <boost/log/core.hpp>//
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>//
#include <boost/log/sinks/text_file_backend.hpp>//
#include <boost/log/utility/setup/file.hpp>//
#include <boost/log/utility/setup/common_attributes.hpp>//
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
/* </> BOOST LOGS */
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
typedef boost::log::sources::severity_logger<boost::log::trivial::severity_level> BOOSTLOGGER; // shortcut for logger define

typedef std::shared_ptr<boost::asio::io_service> SHP_IO;
typedef std::shared_ptr<std::thread> SHP_thread;
/* </> GLOBAL TYPEDEFs */

using namespace boost::log::trivial; // change severity level to my struct.
using namespace std;
