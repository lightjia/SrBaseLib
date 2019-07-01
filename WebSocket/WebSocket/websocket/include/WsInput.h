#ifndef __CWSINPUT__H_
#define __CWSINPUT__H_
#include "UvMutex.h"
#include "wsprotocol.h"
#include "Log.h"
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
    int ParseMsg();

private:
    WS_STATE miState;
	CMemBuffer* mpRecvBuffer;
    CUvMutex mcRecvBufferMutex;
    std::string mstrProtocol;
    CWsCli* mpCli;
    tagWsMsg* mpMsg;
    tagWsMsgCache mstMsgCache;
};

#endif