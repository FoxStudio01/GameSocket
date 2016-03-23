//
//  IButton.h
//  test
//
//  Created by fox on 12-11-19.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef _NETIOBUFFER_H_
#define _NETIOBUFFER_H_

#include "netConfig.h"


namespace FOXSDK
{
	typedef std::vector< fbyte > netDataStream;  

    class netIOBuffer
    {
    public:
        
		netIOBuffer();
		~netIOBuffer();

        fvoid       InitBuffer( fint32 max );
        fvoid       ReleaseBuffer();
        
		fvoid		Write( fint32 l );
        fvoid       Write( fbyte* c , fint32 l );
        fvoid       Read( fvoid** b , fint32 l );
        fint32      ReadRemove( fvoid* b );
        
        fvoid       RemoveBuffer( fint32 l );
        
        
        fint32      GetLen();
        fint32      GetSpace();
        fvoid*      GetBuffer();
        fvoid*      GetStart();
        fvoid*      GetBufferEnd();
        
        fvoid       ClearBuffer();
        
        
    protected:
        
        boost::mutex       mMutex;
        
        fint32      mBegin;
        fint32      mLen;
        
        fbyte*      mBuffer;
        fint32      mMax;
    };
    

}


#endif



