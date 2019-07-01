#ifndef __CWSUPGRADEHANDLER__H_
#define __CWSUPGRADEHANDLER__H_
#include "WsHandler.h"
class CWsUpgradeHandler :  public CWsHandler{
public:
    CWsUpgradeHandler();
    ~CWsUpgradeHandler();

public:
	CMemBuffer* ProcMsg(CWsMsg* pMsg);
};

#endif