#ifndef __CWSHANDLER__H_
#define __CWSHANDLER__H_
#include "WsMsg.h"

class CWsHandler
{
public:
    CWsHandler();
    virtual ~CWsHandler();

public:
    virtual int ProcMsg(CWsMsg* pMsg) = 0;
};

#endif