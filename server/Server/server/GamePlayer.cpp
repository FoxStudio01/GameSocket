
#include "GamePlayer.h"
#include "CreatureManager.h"


GamePlayer::GamePlayer()
:	socket( F_NULL )
{
}

GamePlayer::~GamePlayer()
{

}

fvoid					GamePlayer::SendMsg()
{
    if ( !socket )
    {
        return;
    }
    
	socket->SendMsg();
}


fvoid					GamePlayer::PackMsg( netMsgHead* msg )
{
    if ( !socket )
    {
        return;
    }
    
	socket->PackMsg( msg );
}

fvoid					GamePlayer::SetSocket( netSocket* s )
{
	socket = s;
}

netSocket*              GamePlayer::GetSocket()
{
	return socket;
}

fvoid                   GamePlayer::ReadyClose()
{
    if ( !socket )
    {
        return;
    }
    
    socket->ReadyClose();
}

fvoid					GamePlayer::Init()
{
}

fvoid					GamePlayer::Release()
{
	socket = F_NULL;
}

