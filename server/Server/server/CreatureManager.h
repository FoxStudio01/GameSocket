#ifndef _OBJECTMANAGER_H_
#define _OBJECTMANAGER_H_

#include "GamePlayer.h"
#include "netSocket.h"


// max player
#define PLAYER_CACHE_COUNT 32 



typedef map< netSocket* , GamePlayer* > GamePlayerSocketsMap;
typedef map< netSocket* , GamePlayer* >::iterator GamePlayerSocketsMapIter;


class CreatureManager : public baseSingle< CreatureManager >
{
public:

	CreatureManager();
	~CreatureManager();


	fvoid							Init();

	fvoid							Release();


	GamePlayer*						CreatPlayer();

	// net use
	GamePlayer*						GetPlayer( netSocket* socket );

	fvoid							ReleasePlayer( GamePlayer* player );
	fvoid							RemovePlayer( GamePlayer* player );

	fbool							SetSocket( netSocket* socket , GamePlayer* player );
	fvoid							RemoveSocket( GamePlayer* player );

	fvoid							PackMsgToAll( netMsgHead* msg );
	fvoid							SendMsgToAll();

	fint32							GetFreePlayerCount();
	fint32							GetSocketPlayerCount();



protected:
private:


	GamePlayer*						playerCache[ PLAYER_CACHE_COUNT ];
	GamePlayerList                  freePlayer;
	GamePlayerSocketsMap            playerSocketsMap;

};

extern CreatureManager* gCreatureManager;

#endif

