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
    int ProcMsg(CWsMsg* pMsg);
};

#endif