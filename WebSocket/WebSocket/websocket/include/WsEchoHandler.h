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
    CMemBuffer* ProcMsg(CWsMsg* pMsg);
};

#endif