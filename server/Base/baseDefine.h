//
//  baseDefine.h
//  test
//
//  Created by fox on 12-11-29.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef _BASEDEFINE_H_
#define _BASEDEFINE_H_


#ifdef WIN32
#include <stdio.h>

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <mswsock.h>
#include <Windows.h>
#include <iostream>

#else
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdarg.h>
#endif

#include "baseSingle.h"

#include <time.h>
#include <vector>
#include <list>
#include <string>
#include <queue>
#include <map>
using namespace std;


#define F_TRUE 1
#define F_FALSE 0
#define F_NULL 0
    
#define F_SAFE_DELETE(a) if( a ){ delete a; }
#define F_SAFE_RELEASE(a) if( a ){ a->Release(); }
    
#define F_USE_PRINT
    

#define MEMCPY memcpy
#define MEMSET memset

#define STRLEN strlen
#define STRCMP strcmp
#define STRCPY strcpy
#define ATOF atof
#define ATOI atoi
#define SPRINTF sprintf


#define RAND rand

#ifdef ASSERT
#undef ASSERT
#endif 
#define ASSERT assert

//#define IOS_SERVER
#define ANDROID_SERVER

#define MAX_INT16 65535

//#include "fileTools.h"

#define XMLATTR( a ) "<xmlattr>."#a


#ifdef WIN32
#define SLEEP( n )	Sleep( n )
#else
#define SLEEP( n )	usleep( n * 1000 )
#endif

#define SPLIT_END ";"
#define SPLIT	","

namespace FOXSDK
{
 
    
#ifdef WIN32
    typedef __int64                     fint64;
	#define SNPRINTF _snprintf_c
#else
    typedef int64_t                     fint64;
	#define SNPRINTF snprintf
#endif
    typedef unsigned long long          fuint64;
    
    typedef wchar_t						fwchar;
    
    typedef char						fbyte;
    typedef unsigned char				fubyte;
    
    typedef short						fint16;
    typedef unsigned short				fuint16;
    
    typedef int							fint32;
    typedef unsigned int				fuint32;
    
    typedef int                         flong;
    typedef unsigned int				fulong;
    
    typedef void						fvoid;
    
    typedef float						freal32;
    typedef double						freal64;
    
    typedef int							fbool;
    
    
    
    struct netMsgHead
    {
        fuint16 size;
        fuint16 type;
    };

    
	static const fbyte* LTOA( long l )
	{
		static fbyte buffer[ 64 ];
        printf( "sss%s\n" , buffer );
		sprintf( buffer , "%ld" , l );
		return buffer;
	}
    
	static const fbyte* ITOA( fint32 i )
	{
		static fbyte buffer[ 32 ];
		sprintf( buffer , "%d" , i );
		return buffer;
	}
    
	static const fbyte* FTOA( freal32 f )
	{
		static fbyte buffer[ 32 ];
		sprintf( buffer , "%f" , f );
		return buffer;
	}
    
	// BKDR Hash Function
	static fint64 BKDRHash( const fbyte*str )
	{
		fuint64 seed = 1313; // 31 131 1313 13131 131313 etc..
		fuint64 hash = 0;

		while (*str)
		{
			hash = hash * seed + (*str++);
		}

		return (hash & 0x7FFFFFFFFFFFFFFF);
	}

	static fint32 GetTime()
	{
		time_t ptr;
		time(&ptr);
		return (fint32)ptr;
	}

	static fint32 GetHour( fint32 i )
	{
		return ( ( i + 28800 ) / 3600 ) % 24;
	}

	static fint32 GetDay( fint32 i )
	{
		return ( i + 28800 ) / 86400;
	}

	static fint32 GetWeek( fint32 i )
	{
		return ( i + 28800 - 86400 * 4 ) / 86400 / 7;
	}

	static fubyte GetNo( fint32 i )
	{
		time_t a = (time_t)i;
		tm* t = localtime(&a);
		if(t)
		{
			return (fubyte)t->tm_mday;
		}
		else
		{
			return 1;
		}
	}


	static fint32 GetYear(fint32 i)
	{
		time_t a = (time_t)i;
		tm* t = localtime(&a);
		if (t)
		{
			return t->tm_year + 1900;
		}
		else
		{
			return 1900;
		}
	}

	static fubyte GetMonth(fint32 i)
	{
		time_t a = (time_t)i;
		tm* t = localtime(&a);
		if (t)
		{
			return (fubyte)t->tm_mon + 1;
		}
		else
		{
			return 1;
		}
	}

	static vector<string> SplitStr(const string& str,const string& pattern)
	{
		string::size_type pos = 0;
		vector<string> result;

		string::size_type last = 0;
		pos = str.find(pattern,pos);
		if (pos != string::npos)
		{
			result.push_back(str.substr(last,pos - last));
			last = pos + pattern.size();
			pos++;
		}
		else
		{
			result.push_back(str);
			return result;
		}

		for (;true; pos++)
		{
			pos = str.find(pattern,pos);

			if (pos == string::npos)
			{
				if ((last - str.size()) != 0)
				{
					result.push_back(str.substr(last,str.size() - last));
				}

				break;
			}


			result.push_back(str.substr(last,pos - last));
			last = pos + pattern.size();
		}

		return result;
	}

};



#endif





