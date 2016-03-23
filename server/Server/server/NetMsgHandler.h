#ifndef _NETMSGHANDLER_H_
#define _NETMSGHANDLER_H_

#include "gameDefine.h"
#include "GameError.h"
#include "netSocket.h"

#include "GamePlayer.h"

typedef fint32( *netMsgHandlerf )( GamePlayer* player , netMsgHead* head );

struct NetMsgHandler
{
    fint32 type;
    netMsgHandlerf f;
    string des;
};


fbool			CheckMsg( netMsgHead* msg , fint32 size );

#define CHECKMSG( classType ) \
classType* msg = (classType*)head; \
if ( !CheckMsg( msg , sizeof( classType ) ) ) \
return GAME_ERROR_NETMSG_SIZE;

fvoid			OnNetMsgEnter( netSocket* socket );
fvoid			OnNetMsgExit( netSocket* socket );
fvoid           OnNetMsg( netSocket* socket , netMsgHead* head );

fvoid			OnError( fint32 error , fint32 ServiceID , netSocket* socket , const fbyte* buffer );

fvoid			InitNetMsg();




#endif

