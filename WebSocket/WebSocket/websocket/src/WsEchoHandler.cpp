#include "WsEchoHandler.h"

CWsEchoHandler::CWsEchoHandler(){
}

CWsEchoHandler::~CWsEchoHandler(){
}

len_str CWsEchoHandler::ProcMsg(CWsMsg* pMsg) {
    LOG_INFO("Enter CWsEchoHandler::ProcMsg");
    len_str lRet;
    memset(&lRet, 0, sizeof(lRet));
    ASSERT_RET_VALUE(pMsg, lRet);
    if (pMsg->GetCli() && pMsg->GetMsg()) {
        LOG_INFO("Echo Recv MsgLen:%ld Msg:%s", pMsg->GetMsg()->payloadLength, pMsg->GetMsg()->payload);
        lRet = sWsMsgParse->EncodeMsg(pMsg->GetMsg()->payload, pMsg->GetMsg()->payloadLength, WS_FRAME_TEXT);
    } else {
        LOG_ERR("No Msg or No Cli");
    }

    return lRet;
}