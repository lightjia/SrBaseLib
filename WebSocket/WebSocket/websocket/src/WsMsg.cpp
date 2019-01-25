#include "WsMsg.h"

CWsMsg::CWsMsg(std::string& strProtocol, CWsCli* pCli){
    mstrProtocol = strProtocol;
    mpCli = pCli;
    REF(mpCli);
    mpMsg = NULL;
}

CWsMsg::~CWsMsg(){
    UNREF(mpCli);
    if (mpMsg) {
        DOFREE(mpMsg->payload);
        DOFREE(mpMsg);
    }
}

void CWsMsg::SetMsg(tagWsMsg* pMsg) {
    ASSERT_RET(pMsg);
    mpMsg = pMsg;
}

CWsCli* CWsMsg::GetCli() {
    return mpCli;
}