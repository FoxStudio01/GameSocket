//
//  socket.h
//  test
//
//  Created by fox on 12-11-19.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef _NETSOCKET_H_
#define _NETSOCKET_H_


#include "netIOBuffer.h"


namespace FOXSDK
{
    class netServer;
    

	enum SocketRecvType
	{
		FSRS_NULL = 0,
		FSRS_HEAD = 1,
		FSRS_BODY = 2,
		FSRS_COUNT = 3
	};
    
	enum  MsgReadStatus
	{
		MSG_INVALID		= 0 ,
		MSG_WAITTING	,
		MSG_OK			,
		MSG_REVCING		,
	};

    class netService;
    class netSocket : public tcp::socket
    {
    public:
        
        friend class netServer;
		friend class netClient;

        netSocket( io_service& io_service );
        
		fvoid	InitBuffer( fint32 i , fint32 o );

		fvoid   RecvMsgWeb( const boost::system::error_code& ec, size_t bytes_transferred );
		fvoid   RecvMsgWeb1( fint32 num_bytes , fubyte fin_rsv_opcode , const boost::system::error_code& ec, size_t bytes_transferred );
		fvoid   RecvMsgWebBody( fint32 length , unsigned char fin_rsv_opcode );
		fvoid   RecvMsgWebBody1( fint32 length , unsigned char fin_rsv_opcode , const boost::system::error_code& ec , size_t bytes_transferred );
		fvoid   RecvMsg( const boost::system::error_code& ec, size_t bytes_transferred );
        fvoid   SendMsg( const boost::system::error_code& ec, size_t bytes_transferred );

        fbool   PackMsg( netMsgHead* head );
		fbool	PackMsgWeb( netMsgHead* haed );
        fvoid   SendMsg();
        
        fint32  ReadMsg( netMsgHead** head );
        fvoid   RemoveMsg( int len );

        fint32  IsVaild()
        {
            return mVaild;
        }
   		
        fvoid	ReadyClose();

        fint32	IsClose()
		{
			return mClose;
		}
        
    protected:
        
		fvoid   Close()
		{
			mClose = 1;
		}

        deadline_timer mTimer;
		deadline_timer mCloseTimer;

        virtual ~netSocket();
        
        fvoid   Run();
		fvoid   RunWeb();
                
        fvoid   Clear();
                
        fint32  mVaild;
        
		fvoid   ReadHeadWeb();
        fvoid   ReadHead();
		fvoid   ReadBody();
        
        void    HandleWait( const boost::system::error_code& error );
		void    HandleClose( const boost::system::error_code& error );

        netService*  mService;
        
        netIOBuffer mIBuffer;
        netIOBuffer mOBuffer;
        
        mutable_buffers_1 mHeadBuffer;
        mutable_buffers_1 mBodyBuffer;
        mutable_buffers_1 mSendBuffer1;
        
        fbyte   mRecvBuffer[ MAX_SOCKET_BUFFER ];
        fint32  mRecvStage;
        fint32  mBodyLen;
        
        fbyte   mSendBuffer[ MAX_SOCKET_BUFFER ];
        fint32  mSend;
        
        fbyte	mClose;
		fbyte	mIsReadyClose;

		// use web socket
		map< string , string> header;
		string method , path , httpVersion;
    };
    


    typedef list<netSocket*> SocketList;
    typedef list<netSocket*>::iterator SocketListIter;
    typedef vector<netSocket*> SocketVector;
    typedef vector<netSocket*>::iterator SocketVectorIter;
    

	class netService : public io_service
	{
	public:
		virtual fvoid   SetAccept( netSocket* socket ) = 0;
		virtual fbool	IsWebSocket()
		{
			return F_FALSE;
		}
	};
    
}



#endif
