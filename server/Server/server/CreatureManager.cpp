
#include "CreatureManager.h"


CreatureManager* gCreatureManager = F_NULL;

CreatureManager::CreatureManager()
{
	gCreatureManager = this;
}


CreatureManager::~CreatureManager()
{

}

fvoid				CreatureManager::Init()
{
	for ( fint32 i = 0 ; i < PLAYER_CACHE_COUNT ; i++ )
	{
		playerCache[ i ] = new GamePlayer();
		freePlayer.push_back( playerCache[ i ] );
	}

}


fvoid				CreatureManager::Release()
{
	for ( fint32 i = 0 ; i < PLAYER_CACHE_COUNT ; i++ )
	{
		delete playerCache[ i ];
	}

	delete mThis;
	mThis = F_NULL;
}

GamePlayer*			CreatureManager::CreatPlayer()
{
	if ( freePlayer.empty() )
	{
		return F_NULL;
	}

	GamePlayer* player = *freePlayer.begin();
	freePlayer.erase( freePlayer.begin() );

	//init player 
	player->Init();

	return player;
}

GamePlayer*			CreatureManager::GetPlayer( netSocket* socket )
{
	GamePlayerSocketsMapIter iter = playerSocketsMap.find( socket );

	if ( iter == playerSocketsMap.end() )
	{
		//logManager::Print( "GetPlayer( fint32 guid ) Player not found" );

		return F_NULL;
	}
	else
	{
		return iter->second;
	}
}

fvoid				CreatureManager::RemovePlayer( GamePlayer* player )
{
	GamePlayerListIter iter = std::find( freePlayer.begin() , freePlayer.end() , player );
	if ( iter == freePlayer.end() )
	{
		freePlayer.push_back( player );
	}
}

fbool				CreatureManager::SetSocket( netSocket* socket , GamePlayer* player )
{
	if ( playerSocketsMap.find( socket ) == playerSocketsMap.end() )
	{
		playerSocketsMap[ socket ] = player;

		player->SetSocket( socket );
		return F_TRUE;
	}
	else
	{
		//logManager::Print( "player set socket again" );
		socket->ReadyClose();
		return F_FALSE;
	}
}

fvoid				CreatureManager::RemoveSocket( GamePlayer* player  )
{
	netSocket* socket = player->GetSocket();

	if ( socket )
	{
		GamePlayerSocketsMapIter iter = playerSocketsMap.find( socket );
		playerSocketsMap.erase( iter );
		player->SetSocket( F_NULL );
	}
}

fvoid				CreatureManager::PackMsgToAll( netMsgHead* msg )
{
	GamePlayerSocketsMapIter iterEnd = playerSocketsMap.end();

	for ( GamePlayerSocketsMapIter iter = playerSocketsMap.begin(); iter != iterEnd ; ++iter )
	{
		iter->second->PackMsg( msg );
	}
}

fvoid				CreatureManager::SendMsgToAll()
{
	GamePlayerSocketsMapIter iterEnd = playerSocketsMap.end();

	for ( GamePlayerSocketsMapIter iter = playerSocketsMap.begin(); iter != iterEnd ; ++iter )
	{
		iter->second->SendMsg();
	}
}

fint32				CreatureManager::GetFreePlayerCount()
{
	return (fint32)freePlayer.size();
}

fint32				CreatureManager::GetSocketPlayerCount()
{
	return (fint32)playerSocketsMap.size();
}

