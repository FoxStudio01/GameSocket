#ifndef _GAMEPLAYER_H_
#define _GAMEPLAYER_H_

#include "netSocket.h"
#include "gameDefine.h"

class GamePlayer 
{
public:

	GamePlayer();
	virtual ~GamePlayer();

	virtual fvoid           SendMsg();
	virtual fvoid           PackMsg( netMsgHead* msg );

	fvoid					SetSocket( netSocket* socket );
	netSocket*              GetSocket();
    fvoid                   ReadyClose();

	virtual fvoid			Init();
	fvoid					Release();

protected:

	netSocket*              socket;
};


typedef list< GamePlayer* > GamePlayerList;
typedef list< GamePlayer* >::iterator GamePlayerListIter;

#endif

