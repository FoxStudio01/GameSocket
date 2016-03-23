#ifndef _GAMEMSGDEFINE_H_
#define _GAMEMSGDEFINE_H_


#include "gameDefine.h"
#include "msgType.h"




#pragma pack (1)


struct RECV_MSG_HEART : public netMsgHead
{
	RECV_MSG_HEART()
	{
		size = sizeof(*this);
		type = _MSG_RECV_HEART;
	}
};

struct SEND_MSG_HEART : public netMsgHead
{
	SEND_MSG_HEART()
	{
		size = sizeof(*this);
		type = _MSG_SEND_HEART;
	}
};


//////////////////////////////////////////////////////////////////////////
// chat


struct SEND_RECV_MSG_CHAT_MSG : public netMsgHead
{
	SEND_RECV_MSG_CHAT_MSG()
	{
		size = sizeof(*this);
		type = _MSG_SEND_CHAT_MSG;
	}

	fint32		ChatType;
	fbyte		Name[ MAX_NAME ];
	fbyte		Chat[ MAX_CHAT_MSG ];
};




#pragma pack ()

#endif



