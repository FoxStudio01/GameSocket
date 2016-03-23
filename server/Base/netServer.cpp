//
//  server.cpp
//  test
//
//  Created by fox on 12-11-20.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "netServer.h"
#include <boost/asio.hpp>
//#include <boost/asio/spawn.hpp>
#include <boost/regex.hpp>
#include "baseCrypto.h"

const string WSMagicString = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

using namespace FOXSDK;



netServer::netServer()
:   mAcceptor( *this ) , mSocketMax( 0 ) , 
	mHeadBuffer( buffer( mRecvBuffer , 2048 ) )
{

}


netServer::~netServer()
{

}

fvoid			netServer::Init( fint32 m , fint32 i , fint32 o , fbool web )
{
	mUseWebSocket = web;
	mSocketMax = m;
	mSocket = new netSocket*[ m ];
	for ( int j = 0 ; j < m ; j++ )
	{
		mSocket[ j ] = new netSocket( *this );
		mSocket[ j ]->InitBuffer( i , o );
		mFreeSocket.push_back( mSocket[ j ] );
	}
}

netSocket*      netServer::GetFreeSocket()
{
	if ( mFreeSocket.empty() )
	{
		return NULL;
	}

	netSocket* socket = *mFreeSocket.begin();
	mFreeSocket.erase( mFreeSocket.begin() );

	return socket;
}


fvoid       netServer::SetAccept( netSocket* socket )
{
	mAcceptor.async_accept( *socket ,
		boost::bind( &netServer::HandleAccept , this , boost::asio::placeholders::error ,
		socket ) );
}


fvoid       netServer::SetAddress( const char* ip , unsigned short port )
{
	boost::system::error_code ec;
	mServerAddr = tcp::endpoint( address::from_string( ip , ec ) , port );
	ASSERT(!ec);
}

fvoid       netServer::SetHandler( netServerHandler enter , netServerHandler exit , netServerSHandler msg )
{
	OnEnter = enter;
	OnExit = exit;
	OnMsg = msg;
}


fvoid       netServer::ServerRun()
{
	while ( true )
	{
		boost::system::error_code ec;

		try
		{
			run( ec );
		}
		catch ( ... )
		{
		}
	}
}


fvoid       netServer::ServerUpdate()
{

}


fvoid       netServer::Start()
{
	thread t( boost::bind( &netServer::HandleStart , this ) );
	this_thread::yield();
	t.swap( mServiceThread );
}


fvoid       netServer::HandleStart()
{
	boost::system::error_code ec;
	mAcceptor.open( mServerAddr.protocol(), ec );
	ASSERT( !ec );
	mAcceptor.set_option(tcp::acceptor::reuse_address(true) , ec );
	ASSERT( !ec );
	mAcceptor.bind( mServerAddr , ec );
	ASSERT( !ec );
	mAcceptor.listen( socket_base::max_connections , ec );
	ASSERT( !ec );

	for ( int i = 0 ; i < mSocketMax ; ++i )
	{
		SetAccept( mSocket[ i ] );
	}

	thread_group tg;
	for ( int i = 0; i < MAX_THREAD ; ++i )
	{
		tg.create_thread(boost::bind( &netServer::ServerRun , this ) );
	}

	this_thread::yield();
	tg.join_all();

	//ServerUpdate();
}


fvoid       netServer::Stop()
{

}


fvoid       netServer::Update()
{
	size_t size = mAcceptSocket.size();

	if ( size )
	{
        boost::mutex::scoped_lock lock( mClientListMutex );

		size = mAcceptSocket.size();

		for ( size_t i = 0 ; i < size ; ++i )
		{
			SocketVectorIter iter = std::find( mUsedSocket.begin() , mUsedSocket.end() , mAcceptSocket[i] );
			if ( iter != mUsedSocket.end() )
			{
				(*OnExit)( *iter );
				mUsedSocket.erase( iter );

				FLOG0( "OnNetMsgExit size=%d" , mUsedSocket.size() );
			}

			(*OnEnter)( mAcceptSocket[i] );

			mUsedSocket.push_back( mAcceptSocket[i] );
			FLOG0( "OnNetMsgEnter size=%d " , mUsedSocket.size() );
		}

		mAcceptSocket.clear();

		lock.unlock();
	}

	size = mUsedSocket.size();

	for ( size_t i = 0 ; i < size ; ++i )
	{
		if ( mUsedSocket[ i ]->mIsReadyClose )
		{
			(*OnExit)( mUsedSocket[ i ] );

			mUsedSocket.erase( mUsedSocket.begin() + i );

			i--;
			size--;

			FLOG0( "OnNetMsgExit size=%d" , mUsedSocket.size() );

			continue;
		}
		
		netMsgHead* head = NULL;

		int b = mUsedSocket[ i ]->ReadMsg( &head );


		switch ( b )
		{
		case MSG_INVALID:
			{
				(*OnExit)( mUsedSocket[ i ] );

				mUsedSocket.erase( mUsedSocket.begin() + i );

				i--;
				size--;

				FLOG0( "OnNetMsgExit size=%d" , mUsedSocket.size() );
			}
			break;
		case MSG_OK:
			{
				(*OnMsg)( mUsedSocket[ i ] , head );

				mUsedSocket[ i ]->RemoveMsg( head->size );
			}
			break;
		default:
			break;
		}

	}

}


fvoid       netServer::HandleAccept( const boost::system::error_code& error , netSocket* socket )
{
	if ( error )
	{
		socket->ReadyClose();
	}
	else
	{
		// on enter

		if ( mUseWebSocket )
		{
			ReadHandshake( socket );
		}
		else
		{
			boost::mutex::scoped_lock lock( mClientListMutex );

			//        SocketVectorIter iter = std::find( mUsedSocket.begin() , mUsedSocket.end() , socket );
			//        if ( iter != mUsedSocket.end() )
			//        {
			//            OnExit();
			//            mUsedSocket.erase( iter );
			//        }


			mAcceptSocket.push_back( socket );

			socket->mClose = 0;
			socket->Run();

			lock.unlock();
		}
	}
}

fvoid		netServer::OnReadHandshake( netSocket* socket , const boost::system::error_code& ec, size_t bytes_transferred )
{
	if( !ec ) 
	{
		//Convert to istream to extract string-lines
		std::istream stream( &mReadBuffer );

		ParseHandshake( socket , stream );
		WriteHandshake( socket );
	}
	else
	{
		socket->ReadyClose();
	}
}

fvoid		netServer::ReadHandshake( netSocket* socket ) 
{
	//Create new read_buffer for async_read_until()
	//Shared_ptr is used to pass temporary objects to the asynchronous functions
	
	boost::asio::async_read_until( *socket , mReadBuffer , "\r\n\r\n",
		boost::bind( &netServer::OnReadHandshake , this , socket , boost::asio::placeholders::error , boost::asio::placeholders::bytes_transferred ) );
}

fvoid		netServer::ParseHandshake( netSocket* socket , std::istream& stream ) const {
	std::string line;
	getline(stream, line);
	size_t method_end;
	if((method_end=line.find(' '))!=std::string::npos) {
		size_t path_end;
		if((path_end=line.find(' ', method_end+1))!=std::string::npos) {
			socket->method=line.substr(0, method_end);
			socket->path=line.substr(method_end+1, path_end-method_end-1);
			if((path_end+6)<line.size())
				socket->httpVersion=line.substr(path_end+6, line.size()-(path_end+6)-1);
			else
				socket->httpVersion="1.1";

			getline(stream, line);
			size_t param_end;
			while((param_end=line.find(':'))!=std::string::npos) {
				size_t value_start=param_end+1;
				if((value_start)<line.size()) {
					if(line[value_start]==' ')
						value_start++;
					if(value_start<line.size())
						socket->header.insert(std::make_pair(line.substr(0, param_end), line.substr(value_start, line.size()-value_start-1)));
				}

				getline(stream, line);
			}
		}
	}
}

fvoid		netServer::OnWriteHandshake( netSocket* socket , const boost::system::error_code& ec, size_t bytes_transferred )
{
	if( !ec )
	{
		boost::mutex::scoped_lock lock( mClientListMutex );

		mAcceptSocket.push_back( socket );

		socket->mClose = 0;
		socket->RunWeb();

		lock.unlock();
	}
	else
	{
		socket->ReadyClose();
	}
}

fvoid		netServer::WriteHandshake( netSocket* socket ) 
{
	if( GenerateHandshake( socket ) ) 
	{
		async_write( *socket , mHeadBuffer , 
			transfer_exactly( strlen(mRecvBuffer) ) , 
			boost::bind( &netServer::OnWriteHandshake , this , socket , boost::asio::placeholders::error , boost::asio::placeholders::bytes_transferred )  ) ;
	}
}


fbool		netServer::GenerateHandshake( netSocket* socket ) const 
{
	if( socket->header.count( "Sec-WebSocket-Key" ) == 0 )
		return F_FALSE;

	string sha1 = Crypto::SHA1( socket->header[ "Sec-WebSocket-Key" ] + WSMagicString );

	string str = "HTTP/1.1 101 Web Socket Protocol Handshake\r\n"; 
	str += "Upgrade: websocket\r\n";
	str += "Connection: Upgrade\r\n";
	str += "Sec-WebSocket-Accept: ";
	str += Crypto::Base64::encode( sha1 );
	str += "\r\n\r\n";

	STRCPY( (char*)mRecvBuffer , str.c_str() );

	return F_TRUE;
}

