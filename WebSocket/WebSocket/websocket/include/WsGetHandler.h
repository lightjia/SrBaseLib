#ifndef __CWSGETHANDLER__H_
#define __CWSGETHANDLER__H_
#include "WsHandler.h"
#include "UvMutex.h"
class CWsGetHandler : public CWsHandler{
public:
    CWsGetHandler();
    ~CWsGetHandler();

public:
    int ProcMsg(CWsMsg* pMsg);

private:
    bool mbLoadHtml;
    static len_str mlHtml;
    CUvMutex mcHtmlMux;
};

#endif