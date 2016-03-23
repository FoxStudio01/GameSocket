#include "GameCommand.h"

#include "CreatureManager.h"

#ifdef WIN32 

#include "conio.h"
#else

int _kbhit()
{
    struct timeval tv;
    fd_set rdfs;
    
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    
    FD_ZERO( &rdfs );
    FD_SET ( STDIN_FILENO , &rdfs );
    
    ::select( STDIN_FILENO + 1 , &rdfs , NULL , NULL , &tv );
    return FD_ISSET( STDIN_FILENO , &rdfs );
}

int _getch()
{
    return getchar();
}

#endif



VectorCommand GameCommand::mCommands;

GameCommand::GameCommand()
:	mCommandCount( 0 )
{
	
}

GameCommand::~GameCommand()
{

}

fvoid       GameCommand::Init()
{
    mCommands.push_back( KeyCommand("h" , "help" , CommandHelp ) );
	mCommands.push_back( KeyCommand("p" , "player info" , CommandPlayer ) );
}

fvoid		GameCommand::Command( freal32 delta )
{
	static freal32 time = 0.0f;
	time += delta;

	if ( time < 0.2f )
		return;

	time = 0.0f;

	if( _kbhit() )
	{
		fint32 key = _getch();
		switch( key )
		{
#ifdef WIN32
		case 13:			// Enter
#else
        case 10:
#endif
			{
				if( mCommandCount == 0 )
					break;

				ParseCommand( mBuffer );

				// reset command
				mCommandCount = 0;
				memset( mBuffer , 0 , 256 );
			}
			break;
		case 8:				// BackSpace
			{
				if( mCommandCount > 0 )
				{
					printf( "\b \b" );
					mBuffer[mCommandCount] = 0;
					mCommandCount--;
				}
			}
			break;
		case 27:			// ESC
			{
				// clear command
				mCommandCount = 0;
				memset( mBuffer , 0 , 256 );
				printf( "   ...[Cancel]\nCommand: " );
			}
			break;
		default:
			{
				if( mCommandCount >= 255 )
				{
					break;
				}
				mBuffer[ mCommandCount ] = (char)key;
                mBuffer[ mCommandCount + 1 ] = 0;
				mCommandCount++;
				printf( "%c", key );
				
			}
			break;
		}
	}

}


fbool		GameCommand::ParseCommand( fbyte* command )
{
	VectorCommandIter iterEnd = mCommands.end();

	for ( VectorCommandIter iter = mCommands.begin() ; iter != iterEnd ; ++iter )
	{
		if ( strstr( command , iter->id.c_str() ) )
		{
			iter->callBack( command + iter->id.size() );

			return F_TRUE;
		}
	}

	printf( "\nUnknown command.\n" );

	return F_FALSE;
}

fvoid		GameCommand::CommandHelp( fbyte* )
{
	printf("\n");
	VectorCommandIter iterEnd = mCommands.end();

	for ( VectorCommandIter iter = mCommands.begin() ; iter != iterEnd ; ++iter )
	{
		 FLOG4( "%-16s  %-20s%-25s" , iter->id.c_str() , "-" , iter->doc.c_str() );
	}
}

//extern fbool gIsRun;
//
//fvoid		GameCommand::CommandExit( fbyte* )
//{
//	printf("\n");
//
//	gIsRun = F_FALSE;
//
//	printf("\n$");
//}

fvoid		GameCommand::CommandPlayer( fbyte* )
{
	printf("\n");

	fint32 free = gCreatureManager->GetFreePlayerCount();
	fint32 sockets = gCreatureManager->GetSocketPlayerCount();

	FLOG4( "free=%d , sockets=%d " , free , sockets );
}

