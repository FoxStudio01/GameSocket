//
//  server.h
//  test
//
//  Created by fox on 12-11-20.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef _NETSERVER_H_
#define _NETSERVER_H_



#include "netSocket.h"

namespace FOXSDK
{
    typedef fvoid(*netServerHandler)(netSocket* s);
    typedef fvoid(*netServerSHandler)(netSocket* s , netMsgHead* h );
    
    class netServer : public netService
    {
       
        
    public:
        
        friend class netSocket;
        
        netServer();
        virtual ~netServer();

		fvoid				Init( fint32 m , fint32 i , fint32 o , fbool web = F_FALSE );
        
        virtual fvoid		SetAccept( netSocket* socket );
        fvoid				SetAddress( const fbyte* ip , fuint16 port );
        fvoid				SetHandler( netServerHandler enter , netServerHandler exit , netServerSHandler msg );
        
        fvoid				ServerRun();
        fvoid				ServerUpdate();
        
        fvoid				Start();
        fvoid				Stop();
        
        fvoid				Update();
        
        virtual fbool		IsWebSocket()
		{
			return mUseWebSocket;
		}

    protected:
        
        netServerHandler	OnEnter;
        netServerHandler	OnExit;
        netServerSHandler	OnMsg;
        
        fvoid				HandleStart();
        fvoid				HandleAccept( const boost::system::error_code& error , netSocket* socket );
        
		fvoid				ReadHandshake( netSocket* socket );
		fvoid				WriteHandshake( netSocket* socket );
		fvoid				ParseHandshake( netSocket* socket , std::istream& s ) const;
		fbool				GenerateHandshake( netSocket* socket ) const;
		fvoid				OnWriteHandshake( netSocket* socket , const boost::system::error_code& ec, size_t bytes_transferred );
		fvoid				OnReadHandshake( netSocket* socket , const boost::system::error_code& ec, size_t bytes_transferred );

        netSocket*			GetFreeSocket();
        
		mutable_buffers_1	mHeadBuffer;
		fbyte				mRecvBuffer[ 2048 ];
		asio::streambuf		mReadBuffer;

        SocketVector		mFreeSocket;
        SocketVector		mUsedSocket;
        SocketVector		mAcceptSocket;
        
        netSocket**			mSocket;
		fint32				mSocketMax;
        
        tcp::endpoint		mServerAddr;
        tcp::acceptor		mAcceptor;
        
        thread				mServiceThread;
        boost::mutex		mClientListMutex;

		fbool				mUseWebSocket;

		//std::vector< std::pair< boost::regex , Endpoint* > > opt_endpoint;
    };

}





#endif
