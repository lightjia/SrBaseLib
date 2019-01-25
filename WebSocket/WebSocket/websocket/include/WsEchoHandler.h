#ifndef __CECHOHANDLER__H_
#define __CECHOHANDLER__H_
#include "WsHandler.h"
class CWsEchoHandler :
    public CWsHandler
{
public:
    CWsEchoHandler();
    ~CWsEchoHandler();

public:
    len_str ProcMsg(CWsMsg* pMsg);
};

#endif