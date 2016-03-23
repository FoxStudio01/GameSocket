#include "NetChatHandler.h"
#include "NetMsgHandler.h"

#include "CreatureManager.h"
#include "msgDefine.h"



fint32		NetHeartHandler( GamePlayer* player , netMsgHead* head )
{
	head->type = _MSG_RECV_HEART;

	player->PackMsg( head );
	player->SendMsg();

	return GAME_NO_ERROR;
}

fint32		NetChatHandler( GamePlayer* player , netMsgHead* head )
{
    CHECKMSG( SEND_RECV_MSG_CHAT_MSG )

	msg->type = _MSG_RECV_CHAT_MSG;

	gCreatureManager->PackMsgToAll( msg );
	gCreatureManager->SendMsgToAll();

	return GAME_NO_ERROR;
}

