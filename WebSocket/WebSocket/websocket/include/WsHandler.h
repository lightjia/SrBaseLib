#ifndef __CWSHANDLER__H_
#define __CWSHANDLER__H_
#include "WsMsg.h"
#include "WsMsgParse.h"

class CWsHandler{
public:
    CWsHandler();
    virtual ~CWsHandler();

public:
    virtual len_str ProcMsg(CWsMsg* pMsg) = 0;
};

#endif