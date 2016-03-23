#include "ServerConfig.h"
#include "logManager.h"
#include "netSocket.h"


ServerConfig* gServerConfig = F_NULL;

ServerConfig::ServerConfig()
{
	gServerConfig = this;
}


fvoid			ServerConfig::Init( const fbyte* path )
{
    ptree pt;
    
    read_xml( path , pt , trim_whitespace );
    
    FLOG4( "xml loading. %s" , path );
    
    pt = pt.get_child( "config" );
    
	IP = pt.get< string >( XMLATTR(ip) );
	Port = pt.get< fint16 >( XMLATTR(port) );

	FLOG4( "xml loaded. %s" , path );
}

