#pragma once
#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <tchar.h>
#include <SDKDDKVer.h>


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
#include <boost/thread/thread.hpp>
#include <boost/signals2.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/any.hpp>

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
