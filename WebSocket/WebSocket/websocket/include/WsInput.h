#ifndef __CWSINPUT__H_
#define __CWSINPUT__H_
#include "UvMutex.h"
#include "wsprotocol.h"
#include "CLogmanager.h"
#include <queue>

class CWsCli;
class CWsInput{
public:
    CWsInput(CWsCli* pCli);
    ~CWsInput();

public:
    int PushMsg(char* pMsg, ssize_t iLen);
    int ProcMsg();
    void SetProtocol(std::string& strProtocol) { mstrProtocol = strProtocol; }
    void SetState(WS_STATE iState) { miState = iState; }

private:
    int ParseMsg(len_str& lStr);

private:
    WS_STATE miState;
    std::queue<len_str> mqueMsg;
    CUvMutex mcQueMsgMutex;
    std::string mstrProtocol;
    CWsCli* mpCli;
    tagWsMsg* mpMsg;
    tagWsMsgCache mstMsgCache;
};

#endif