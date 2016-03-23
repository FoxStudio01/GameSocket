
#include "NetMsgHandler.h"
#include "NetChatHandler.h"

#include "CreatureManager.h"

#include "netServer.h"
#include "msgDefine.h"


static map< fint32 , NetMsgHandler > gMsgHeandler;

fbool			CheckMsg( netMsgHead* msg , fint32 size )
{
	if ( msg->size == size )
	{
		return F_TRUE;
	}
	else
	{
		return F_FALSE;
	}
}


fvoid			InitNetMsg()
{
	static NetMsgHandler handler[] =
	{
		{ _MSG_SEND_HEART , NetHeartHandler , "" },

		{ _MSG_SEND_CHAT_MSG , NetChatHandler , "" },

		{ 0 , F_NULL , "" }
	};

	
    fint32 i = 0;
	while ( handler[i].type )
	{
		gMsgHeandler[ handler[ i ].type ] = handler[ i ];
        
		i++;
	}
}


fvoid			OnNetMsgEnter( netSocket* socket )
{
	GamePlayer* player = gCreatureManager->GetPlayer( socket );

	if ( player )
	{
        // foxsdk error
        
		ASSERT( 0 );
		socket->ReadyClose();
		return;
	}

	player = gCreatureManager->CreatPlayer();

	if ( player )
	{
		fbool bload = gCreatureManager->SetSocket( socket , player );

		if ( !bload )
		{
			// foxsdk error
            
            ASSERT( 0 );
			socket->ReadyClose();
			return;
		}
		else
		{
			player->Init();
			//FLOG0( "player load %s" , socket->GetIP() );
		}

	}
	else
	{
		// error

		socket->ReadyClose();
	}

}


fvoid           OnNetMsg( netSocket* socket , netMsgHead* head )
{
    GamePlayer* player = gCreatureManager->GetPlayer( socket );
    
    map< fint32 , NetMsgHandler >::iterator iter = gMsgHeandler.find( head->type );
    
    if ( iter == gMsgHeandler.end() )
    {
		FLOG0( "bad player %d %d " , head->type , head->size );
        // bad player,
        //socket->Close();
    }
    else
    {
        (iter->second.f)( player , head );
    }
}



fvoid			OnNetMsgExit( netSocket* socket )
{    
	GamePlayer* player = gCreatureManager->GetPlayer( socket );

	if ( !player )
	{
        // foxsdk error
        
		ASSERT( 0 );
		return;
	}

	gCreatureManager->RemoveSocket( player );

	player->Release();
	gCreatureManager->RemovePlayer( player );
}


