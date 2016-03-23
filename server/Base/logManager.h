//
//  logManager.h
//  test
//
//  Created by fox on 13-1-9.
//
//

#ifndef _LOGMANAGER_H_
#define _LOGMANAGER_H_

#include "baseDefine.h"

namespace FOXSDK
{
    enum logLevel
    {
        LOGLEVEL0 = 0,
        LOGLEVEL1,
        LOGLEVEL2,
        LOGLEVEL3,
        LOGLEVEL4
    };
    
    extern fint32 gLogLevel;
    
    fvoid log( fint32 level , const fbyte* str , ... );
    fvoid logFile( fint32 level , const fbyte* str , ... );
    
#ifdef F_USE_PRINT
	#ifdef WIN32
		#define FLOG( level , fmt , arg ,... ) log( level , fmt , ##arg __VA_ARGS__ );
		#define FLOG0( fmt , ... ) log( LOGLEVEL0 , fmt , __VA_ARGS__ );
		#define FLOG1( fmt , ... ) log( LOGLEVEL1 , fmt , __VA_ARGS__ );
		#define FLOG2( fmt , ... ) log( LOGLEVEL2 , fmt , __VA_ARGS__ );
		#define FLOG3( fmt , ... ) log( LOGLEVEL3 , fmt , __VA_ARGS__ );
		#define FLOG4( fmt , ... ) log( LOGLEVEL4 , fmt , __VA_ARGS__ );

	#else
		#define FLOG( level , fmt , a... ) log( level , fmt , ##a );
        
        #define FLOG0( fmt , a... ) log( LOGLEVEL0 , fmt , ##a );
        #define FLOG1( fmt , a... ) log( LOGLEVEL1 , fmt , ##a );
        #define FLOG2( fmt , a... ) log( LOGLEVEL2 , fmt , ##a );
        #define FLOG3( fmt , a... ) log( LOGLEVEL3 , fmt , ##a );
        #define FLOG4( fmt , a... ) log( LOGLEVEL4 , fmt , ##a );
	#endif

#else
	#ifdef WIN32
	#define FLOG( level , fmt , arg ,... ) logFile( level , fmt  "\n%s %s() line:%d" , ##arg __VA_ARGS__ , __FILE__ , __FUNCTION__ , __LINE__ );
	#else
	#define FLOG( level , fmt , a... ) logFile( level , fmt  "\n%s %s() line:%d" , ##a , __FILE__ , __FUNCTION__ , __LINE__ );
	#endif
    
#endif
    
}






#endif
