#ifndef _SERVERCONFIG_H_
#define _SERVERCONFIG_H_

#include "gameDefine.h"

class ServerConfig : public baseSingle< ServerConfig >
{
public:

	ServerConfig();
	~ServerConfig()
	{

	}

	fvoid			Init( const fbyte* path );

	string			IP;
	fint16			Port;

protected:
private:
};


extern ServerConfig* gServerConfig;

#endif

