//
//  IOBuffer.cpp
//  test
//
//  Created by fox on 12-11-19.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "netIOBuffer.h"



using namespace FOXSDK;


netIOBuffer::netIOBuffer()
:	mBegin( 0 ) , mLen( 0 ) , 
	mBuffer( F_NULL ) , mMax( 0 )
{
	
}

netIOBuffer::~netIOBuffer()
{
	
}

fvoid       netIOBuffer::InitBuffer( fint32 max )
{
    mMax = max;
    
    if ( mBuffer ) 
    {
        delete [] mBuffer;
        mBuffer = NULL;
    }
    
    mBuffer = new char[ max ];
}


fvoid       netIOBuffer::ReleaseBuffer()
{
    if ( mBuffer ) 
    {
        delete [] mBuffer;
        mBuffer = NULL;
    }
}

fvoid		netIOBuffer::Write( fint32 l )
{
	mLen += l;
}

fvoid       netIOBuffer::Write( fbyte* c , fint32 l )
{
    boost::mutex::scoped_lock lock( mMutex );
    
	if ( mBegin > mMax * 0.5f )
	{
		// use half
		memcpy( mBuffer , mBuffer + mBegin , mLen );
		mBegin = 0;
	}

    if ( l > mMax - mBegin - mLen )
    {
		ClearBuffer();

        lock.unlock();
        // error,,
		FLOG4( "netIOBuffer sdk error" );
        return;
    }
    
    memcpy( mBuffer + mBegin + mLen , c , l );
    
    mLen += l;

    lock.unlock();

	return;
}


fvoid       netIOBuffer::Read( fvoid** b , fint32 l )
{
    if ( l > mLen ) 
    {
        // error,,
        return;
    }
    
    *b = mBuffer + mBegin;
}

fint32      netIOBuffer::ReadRemove( fvoid* b )
{
    if ( !mLen ) 
    {
        return mLen;
    }
    
    boost::mutex::scoped_lock lock( mMutex );
    
    int n = mLen;

	if ( n > MAX_SOCKET_BUFFER )
	{
		n = MAX_SOCKET_BUFFER - 32;
		memcpy( b , mBuffer + mBegin , n );
		mBegin += n;
		mLen -= n;
	}
	else
	{
		memcpy( b , mBuffer + mBegin , mLen );
		mBegin = 0;
		mLen = 0;
	}
    
	
    
    lock.unlock();
    
    return n;
}


fvoid       netIOBuffer::RemoveBuffer( fint32 l )
{
    boost::mutex::scoped_lock lock( mMutex );
    
    mBegin += l;
    mLen -= l;
    
    if ( mLen == 0 )
    {
        mBegin = 0;
    }
    
    lock.unlock();
}

fint32      netIOBuffer::GetLen()
{
    return mLen;
}
fint32      netIOBuffer::GetSpace()
{
    return mMax - mBegin - mLen;
}
fvoid*      netIOBuffer::GetBuffer()
{
    return mBuffer;
}
fvoid*      netIOBuffer::GetStart()
{
    return mBuffer + mBegin;
}
fvoid*      netIOBuffer::GetBufferEnd()
{
    return mBuffer + mBegin + mLen;
}

fvoid       netIOBuffer::ClearBuffer()
{
    mBegin = 0;
    mLen = 0;
    memset( mBuffer , 0 , mMax );
}



