#ifndef _GAMECOMMAND_H_
#define _GAMECOMMAND_H_


#include "gameDefine.h"

typedef fvoid	(*CommandFun)( fbyte* );

struct KeyCommand
{
	KeyCommand( const fbyte* id , const fbyte* doc , CommandFun call )
	{
		this->id = id;
		this->doc = doc;
		this->callBack = call;
	}

	string		id;
	string		doc;

	CommandFun	callBack;
};

typedef vector< KeyCommand > VectorCommand;
typedef vector< KeyCommand >::iterator VectorCommandIter;

class GameCommand
{
public:

	GameCommand();
	~GameCommand();
    
    fvoid           Init();

	fvoid			Command( freal32 delta );
	fbool			ParseCommand( fbyte* command );

protected:


	static fvoid	CommandHelp( fbyte* );
	static fvoid	CommandPlayer( fbyte* );

private:

	static VectorCommand	mCommands;

	fbyte			mBuffer[ 256 ];
	fint32			mCommandCount;
};



#endif


