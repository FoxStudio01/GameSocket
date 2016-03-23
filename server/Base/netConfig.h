//
//  netConfig.h
//  test
//
//  Created by fox on 13-1-9.
//
//

#ifndef _NETCONFIG_H_
#define _NETCONFIG_H_


#include "baseDefine.h"
#include "logManager.h"

#include <boost/timer.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/container/set.hpp>
#include <boost/thread/thread.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/typeof/std/utility.hpp>

#include <boost/date_time/local_time/local_time.hpp>

#include <boost/iostreams/filtering_streambuf.hpp>  
#include <boost/iostreams/filtering_stream.hpp>  
#include <boost/iostreams/copy.hpp>  
#include <boost/iostreams/filter/gzip.hpp>  

using namespace boost;
using namespace boost::system;
using namespace boost::asio::ip;
using namespace boost::asio;
using namespace boost::property_tree;
using namespace boost::property_tree::xml_parser;
using namespace boost::gregorian;  
using namespace boost::local_time;  
using namespace boost::posix_time;  
using namespace boost::iostreams;  

#define USE_ZIP

#define MAX_SOCKET_BUFFER 10240
#define MAX_THREAD 8


#endif

