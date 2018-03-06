#pragma once
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
#include <boost/any.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/signals2.hpp>
//#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/circular_buffer.hpp>

#undef BOOST_ASIO_ERROR_CATEGORY_NOEXCEPT 
/* </> BOOST */

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
