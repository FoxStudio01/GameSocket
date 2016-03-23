//
//  socket.cpp
//  test
//
//  Created by fox on 12-11-19.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//


#pragma warning(disable:4244 4996)

#include "netSocket.h"
#include "netServer.h"
#include "logManager.h"

using namespace FOXSDK;

//mutex   netSocket::mMutex;

netSocket::netSocket( io_service& io_service ):tcp::socket ( io_service ) ,
	mHeadBuffer( buffer( mRecvBuffer , sizeof(netMsgHead) ) ),
	mBodyBuffer( buffer( mRecvBuffer + sizeof(netMsgHead) , MAX_SOCKET_BUFFER - sizeof(netMsgHead) ) ) ,
	mSendBuffer1( buffer( mSendBuffer , MAX_SOCKET_BUFFER ) ),
	mBodyLen( 0 ) , mRecvStage( FSRS_NULL ) ,
	mVaild( 0 ) ,
	mSend( 0 ) ,
	mClose( 0 ) ,
	mTimer( io_service ) ,
	mCloseTimer( io_service ) ,
	mIsReadyClose( 0 )
{
	mService = (netService*)&io_service;

	boost::asio::socket_base::linger option( true , 0 );
	boost::system::error_code ec1;
	set_option( option , ec1 );
}

netSocket::~netSocket()
{

}

fvoid	netSocket::InitBuffer( fint32 i , fint32 o )
{
	mIBuffer.InitBuffer( i );
	mOBuffer.InitBuffer( o );
}


fint32      netSocket::ReadMsg( netMsgHead** head )
{
	if ( !mVaild )
	{
		return MSG_INVALID;
	}

	int len = mIBuffer.GetLen();

	if ( len )
	{
		*head = (netMsgHead*)mIBuffer.GetStart();

		if ( (*head)->size > len )
		{
			return MSG_REVCING;
		}

		return MSG_OK;
	}

	return MSG_WAITTING;
}


fvoid       netSocket::RemoveMsg( fint32 len )
{
	mIBuffer.RemoveBuffer( len );
}


fvoid       netSocket::RecvMsgWeb1( fint32 num_bytes , fubyte fin_rsv_opcode , const boost::system::error_code& ec, size_t bytes_transferred )
{
	if( !ec )
	{
		size_t length = 0;
		for( fint32 c = 0 ; c < num_bytes ; c++ )
		{
			fubyte cc = mRecvBuffer[ c + sizeof( netMsgHead ) ];
			length += cc << ( 8 * ( num_bytes - 1 - c ) );
		}

		RecvMsgWebBody( length , fin_rsv_opcode );
	}
	else
	{
		ReadyClose();
	}
}

fvoid       netSocket::RecvMsgWeb( const boost::system::error_code& ec, size_t bytes_transferred )
{
	if ( ec )
	{
		ReadyClose();
		return;
	}

	if ( !mVaild || mClose || mIsReadyClose )
	{
		return;
	}

	mTimer.cancel();

	if( bytes_transferred == 0 )
	{
		ASSERT( 0 );
	}
	
	fubyte fin_rsv_opcode = mRecvBuffer[ 0 ];
	fubyte len = mRecvBuffer[ 1 ];

	// Close connection if unmasked message from client (protocol error)
	if( len < 128 ) 
	{
		ReadyClose();
		return;
	}

	size_t length = ( len & 127 );

	if( length == 126 ) 
	{
		// 2 next bytes is the size of content
		boost::asio::async_read( *this , mBodyBuffer , boost::asio::transfer_exactly( 2 ) ,
			 boost::bind( &netSocket::RecvMsgWeb1 , this , 2 , fin_rsv_opcode , boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) );
	}
	else if( length == 127 )
	{
		// 8 next bytes is the size of content
		boost::asio::async_read( *this , mBodyBuffer , boost::asio::transfer_exactly( 8 ),
			boost::bind( &netSocket::RecvMsgWeb1 , this , 8 , fin_rsv_opcode , boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) );
	}
	else
		RecvMsgWebBody( length , fin_rsv_opcode );

}

fvoid		netSocket::RecvMsgWebBody1( fint32 length , unsigned char fin_rsv_opcode , const boost::system::error_code& ec , size_t bytes_transferred )
{
	if( !ec ) 
	{
		for( fint32 c = 0 ; c < length ; c++ ) 
		{
			mRecvBuffer[ c + 4 + sizeof( netMsgHead ) ] = mRecvBuffer[ c + 4 + sizeof( netMsgHead ) ] ^ mRecvBuffer[ c % 4 + sizeof( netMsgHead ) ];
		}

		mIBuffer.Write( &mRecvBuffer[ 4 + sizeof( netMsgHead ) ] , length );

		// If connection close
		if( ( fin_rsv_opcode & 0x0f ) == 8 ) 
		{
			ReadyClose();
			return;
		}
		else 
		{
			// Next message
			ReadHeadWeb();
		}
	}
	else
	{
		ReadyClose();
	}
}

fvoid		netSocket::RecvMsgWebBody( fint32 length , unsigned char fin_rsv_opcode )
{
	boost::asio::async_read( *this , mBodyBuffer , boost::asio::transfer_exactly( 4 + length ) ,
		boost::bind( &netSocket::RecvMsgWebBody1 , this , length , fin_rsv_opcode , boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) );
}

fvoid       netSocket::RecvMsg( const boost::system::error_code& ec, size_t bytes_transferred )
{
	if ( ec )
	{
		ReadyClose();
		return;
	}

	if ( !mVaild || mClose || mIsReadyClose )
	{
		return;
	}

	mTimer.cancel();

#ifdef DEBUG
	static fint32 count = 0; count++;
	static fint32 count1 = 0;
#endif

	switch ( mRecvStage )
	{
	case (fint32)FSRS_HEAD:
		{
			netMsgHead* head = (netMsgHead*)mRecvBuffer;
			mBodyLen = head->size;

			if ( mBodyLen == 0 )
			{
				ReadyClose();
				return;
			}

			if ( mBodyLen == sizeof( netMsgHead ) )
			{
				mIBuffer.Write( mRecvBuffer , sizeof( netMsgHead ) );
				ReadHead();
#ifdef DEBUG
				count1 += mBodyLen;
				FLOG0( "RecvMsg count=%d total=%.2fk" , count , count1 * 0.001f );
#endif
				return;
			}
			else
			{
				mRecvStage = FSRS_BODY;
				ReadBody();
				return;
			}
		}
		break;
	case (fint32)FSRS_BODY:
		{
#ifdef USE_ZIP
			netDataStream stream;
			stream.reserve( mBodyLen );

			if ( mBodyLen <= sizeof( netMsgHead ) )
			{
				ReadBody();
				return;
			}

			try
			{
				boost::iostreams::filtering_ostream sm;
				sm.push( boost::iostreams::gzip_decompressor() );
				sm.push( boost::iostreams::back_inserter( stream ) );

				boost::iostreams::copy( boost::make_iterator_range( mRecvBuffer + sizeof( netMsgHead ) , mRecvBuffer + mBodyLen ),
					sm );
			}
			catch ( ... )
			{
				ReadyClose();
				return;
			}

			fuint16* ppsize = (fuint16*)mRecvBuffer;
			fuint16 ss = stream.size();
			*ppsize = ss + sizeof( netMsgHead ) ;

			if ( *ppsize > mIBuffer.GetSpace() - 32 )
			{
				FLOG3( "iBuffer not enouth space." );
				mRecvStage = FSRS_HEAD;
				ReadHead();
				return;
			}

			mIBuffer.Write( mRecvBuffer , sizeof( netMsgHead ) );
			mIBuffer.Write( (fbyte*)&stream[ 0 ] , ss );
			mRecvStage = FSRS_HEAD;
			ReadHead();

#ifdef DEBUG
			count1 += mBodyLen;
			FLOG0( "RecvMsg count=%d total=%.2fk" , count , count1 * 0.001f );
#endif

#else
#ifdef DEBUG
			count1 += mBodyLen;
			FLOG0( "RecvMsg count=%d total=%.2fk" , count , count1 * 0.001f );
#endif
			if ( mBodyLen > mIBuffer.GetSpace() - 32 )
			{
				FLOG3( "iBuffer not enouth space." );
				mRecvStage = FSRS_HEAD;
				ReadHead();
				return;
			}

			mIBuffer.Write( mRecvBuffer , mBodyLen );
			mRecvStage = FSRS_HEAD;
			ReadHead();
#endif
			return;
		}
		break;
	default:
		{
			//ASSERT( 0 );
		}
		break;
	}

}


fvoid       netSocket::Clear()
{
	if ( !mVaild )
	{
		return;
	}

	mTimer.cancel();
	mCloseTimer.cancel();

	mVaild = 0;
	mIsReadyClose = 0;

	mRecvStage = FSRS_NULL;

	mBodyLen = 0;

	mIBuffer.ClearBuffer();
	mOBuffer.ClearBuffer();

	mSend = 0;

	header.clear();
	method = "";
	path = "";
	httpVersion = "";

    //FLOG0( "Close" );
}


fbool		netSocket::PackMsgWeb( netMsgHead* head )
{
	if ( !mVaild || mClose || mIsReadyClose )
	{
		return F_FALSE;
	}

	if ( head->size > mOBuffer.GetSpace() - 32 )
	{
		FLOG3( "oBuffer not enouth space." );
		return F_FALSE;
	}

	fubyte fin_rsv_opcode = 130;

	fint16 length = head->size;

	mOBuffer.Write( (fbyte*)&fin_rsv_opcode , 1 );
	
	fubyte tem = 0;

	//unmasked (first length byte<128)
	if( length >= 126 ) 
	{
		fint32 num_bytes;
		if ( length > 0xffff ) 
		{
			num_bytes = 8;
			tem = 127;
			mOBuffer.Write( (fbyte*)&tem , 1 );
		}
		else 
		{
			num_bytes = 2;
			tem = 126;
			mOBuffer.Write( (fbyte*)&tem , 1 );
		}

		for( fint32 c = num_bytes - 1 ; c >= 0 ; c-- )
		{
			tem = ( length >> ( 8 * c ) ) % 256;
			mOBuffer.Write( (fbyte*)&tem , 1 );
		}
	}
	else
	{
		tem = length;
		mOBuffer.Write( (fbyte*)&tem , 1 );
	}
				  
	mOBuffer.Write( (fbyte*)head , head->size );

	return F_TRUE;
}


fbool       netSocket::PackMsg( netMsgHead* head )
{
	if ( mService->IsWebSocket() )
	{
		return PackMsgWeb( head );
	}

	if ( !mVaild || mClose || mIsReadyClose )
	{
		return F_FALSE;
	}

	if ( head->size > mOBuffer.GetSpace() - 32 )
	{
		FLOG3( "oBuffer not enouth space." );
		return F_FALSE;
	}

#ifdef USE_ZIP

	netDataStream stream;
	stream.reserve( head->size );

#ifdef DEBUG
	static fint32 count = 0; count++;
	static fint32 count1 = 0;
#endif

	if ( head->size == sizeof( netMsgHead ) )
	{
		mOBuffer.Write( (fbyte*)head , head->size );
#ifdef DEBUG
		count1 += head->size;
		FLOG0( "PackMsg count=%d total=%.2fk type=%d" , count , count1 * 0.001f , head->type );
#endif
	}
	else
	{
        try
        {
            filtering_ostream sm;
            sm.push( boost::iostreams::gzip_compressor() );
            sm.push( boost::iostreams::back_inserter( stream ) );

            copy( boost::make_iterator_range( (fbyte*)head + sizeof( netMsgHead ) , (fbyte*)head + head->size ),sm );
        }
        catch ( ... )
        {
            ReadyClose();
            return F_FALSE;
        }

		fuint16 size = (fuint16)stream.size();
		fuint16 ss = size + sizeof( netMsgHead );

		mOBuffer.Write( (fbyte*)&ss , 2 );
		mOBuffer.Write( (fbyte*)&head->type , 2 );
		mOBuffer.Write( (fbyte*)&stream[ 0 ], size );

#ifdef DEBUG
		count1 += ss;
		FLOG0( "PackMsg count=%d total=%.2fk type=%d" , count , count1 * 0.001f , head->type );
#endif
	}
#else
	mOBuffer.Write( (fbyte*)head , head->size );
#endif

	return F_TRUE;
}

fvoid       netSocket::SendMsg()
{
	if ( !mVaild || mClose || mIsReadyClose )
	{
		return;
	}

	if ( !mSend )
	{
		int len = mOBuffer.ReadRemove( &mSendBuffer );

		if ( len )
		{
			mSend = 1;

			async_write( *this , mSendBuffer1 , transfer_exactly( len ) , boost::bind( &netSocket::SendMsg , this , boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred )  ) ;
		}
	}
}

fvoid       netSocket::SendMsg( const boost::system::error_code& ec, size_t bytes_transferred )
{
	if ( ec )
	{
		ReadyClose();
		return;
	}

#ifdef DEBUG
	static fint32 count = 0; count++;
	static fint32 count1 = 0; count1 += bytes_transferred;
	FLOG0( "SendMsg count=%d total=%.2fk" , count , count1 * 0.001f );
#endif
	int len = mOBuffer.ReadRemove( &mSendBuffer );

	if ( len )
	{
		async_write( *this , mSendBuffer1 , transfer_exactly( len ) , boost::bind( &netSocket::SendMsg , this , boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred )  ) ;
	}
	else
	{
		mSend = 0;
	}
}

fvoid		netSocket::ReadyClose()
{
	if ( !mVaild || mClose || mIsReadyClose )
	{
		return;
	}

	try
	{
		boost::system::error_code ec;
		shutdown( boost::asio::ip::tcp::socket::shutdown_both , ec );
	}
	catch ( ... )
	{
	}

	mClose = 1;

	mTimer.cancel();
	mCloseTimer.cancel();

	mCloseTimer.expires_from_now( boost::posix_time::seconds( 3 ) );
	mCloseTimer.async_wait( boost::bind( &netSocket::HandleClose , this , boost::asio::placeholders::error ) );

	mIsReadyClose = 1;
}

void		netSocket::HandleClose( const boost::system::error_code& error )
{
	if( error )
	{
		return;
	}

	if ( !mIsReadyClose )
	{
		return;
	}

	try
	{
		boost::system::error_code ec2;
		tcp::socket::close( ec2 );
	}
	catch ( ... )
	{
	}

	Clear();
	mService->SetAccept( this );
}

void        netSocket::HandleWait( const boost::system::error_code& error )
{
    if( error )
    {
        return;
    }

	ReadyClose();
}

fvoid		netSocket::ReadHeadWeb()
{
	async_read( *this , mHeadBuffer ,
		transfer_exactly( 2 ) ,
		boost::bind( &netSocket::RecvMsgWeb , this , boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ) ;

	mTimer.cancel();
	mTimer.expires_from_now( boost::posix_time::seconds( 15 ) );
	mTimer.async_wait( boost::bind( &netSocket::HandleWait , this , boost::asio::placeholders::error ) );
}

fvoid       netSocket::ReadHead()
{
	async_read( *this , mHeadBuffer ,
		transfer_exactly( sizeof(netMsgHead) ) ,
		boost::bind( &netSocket::RecvMsg , this , boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ) ;

	mTimer.cancel();
	mTimer.expires_from_now( boost::posix_time::seconds( 15 ) );
	mTimer.async_wait( boost::bind( &netSocket::HandleWait , this , boost::asio::placeholders::error ) );
}


fvoid       netSocket::ReadBody()
{
	async_read( *this , mBodyBuffer ,
		transfer_exactly( mBodyLen - sizeof(netMsgHead) ) ,
		boost::bind( &netSocket::RecvMsg , this , boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ) ;

	mTimer.cancel();
	mTimer.expires_from_now( boost::posix_time::seconds( 15 ) );
	mTimer.async_wait( boost::bind( &netSocket::HandleWait , this , boost::asio::placeholders::error) );
}


fvoid       netSocket::Run()
{
	mRecvStage = FSRS_HEAD;

	mVaild = 1;

	ReadHead();
}

fvoid       netSocket::RunWeb()
{
	mRecvStage = FSRS_HEAD;

	mVaild = 1;

	ReadHeadWeb();
}


