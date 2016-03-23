//
//  main.cpp
//  test
//
//  Created by fox on 12-11-19.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//




#include "netServer.h"

#include "CreatureManager.h"
#include "NetMsgHandler.h"
#include "GameCommand.h"
#include "ServerConfig.h"

fvoid			RunServerExit();
extern fvoid	InitException();

GameCommand		gGameCommand;
netServer*		gNetServer;
netServer*		gNetServerWeb;

fbool			gIsRun = F_TRUE;
fbool			gIsExit = F_TRUE;


fvoid   RunServerExit()
{
	
}

fvoid   ServerInit()
{
	gGameCommand.Init();

	ServerConfig::Instance()->Init( "server.xml" );
	CreatureManager::Instance()->Init();
   
	InitNetMsg();
    	
	FLOG4( "net Init." );
	FLOG4( "CreatureManager Init." );
	FLOG4( "Loading data...");    

    gNetServer = new netServer();
    gNetServer->Init( 32 , 1024 , 1024 );
    gNetServer->SetAddress( gServerConfig->IP.c_str() , gServerConfig->Port );
    gNetServer->SetHandler( OnNetMsgEnter , OnNetMsgExit , OnNetMsg );
    gNetServer->Start();
    
	gNetServerWeb = new netServer();
	gNetServerWeb->Init( 32 , 1024 , 1024 , F_TRUE );
	gNetServerWeb->SetAddress( gServerConfig->IP.c_str() , gServerConfig->Port + 1 );
	gNetServerWeb->SetHandler( OnNetMsgEnter , OnNetMsgExit , OnNetMsg );
	gNetServerWeb->Start();

    FLOG4( "server started.\n" );
    
    InitException();
    
    SLEEP( 5000 );
}


fvoid   GameRun( freal32 delay )
{
    
    // logic run here ,
    
    gGameCommand.Command( delay );
}


fvoid   ServerRun()
{
    timer t;
    freal32 now = 0;
    freal32 last = 0;
    
    srand( GetTime() );
	
    fint32 count = 0;
    
    
    while ( gIsRun )
    {
        if ( count > 10000 )
        {
			SLEEP( 300 );
            count = 0;
        }
        
        count++;
        
        gNetServer->Update();
		gNetServerWeb->Update();
                
        now = (freal32)t.elapsed();
        
        GameRun( now - last );
        
        last = now;
        
    }
}


int main(int argc, const char * argv[])
{
    ServerInit();
    
    ServerRun();
    
    RunServerExit();
    
    return 0;
}

