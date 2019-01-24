#ifndef __CWSHANDLERMGR__H_
#define __CWSHANDLERMGR__H_
#include "singleton.h"
#include "WsHandler.h"
#include <map>
class CWsHandlerMgr : public CSingleton<CWsHandlerMgr>{
    SINGLE_CLASS_INITIAL(CWsHandlerMgr);
public:
    ~CWsHandlerMgr();

public:
    int Init();
    int UnInit();
    int ProcMsg(CWsMsg* pMsg);

private:
    CWsHandler* GetHandler(std::string& strProtocol);

private:
    std::map<std::string, CWsHandler*> mmapHandlers;
    bool mbInit;
};
#define sWsHandlerMgr CWsHandlerMgr::Instance()
#endif