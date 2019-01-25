#include "WsEchoHandler.h"

CWsEchoHandler::CWsEchoHandler(){
}

CWsEchoHandler::~CWsEchoHandler(){
}

int CWsEchoHandler::ProcMsg(CWsMsg* pMsg) {
    LOG_INFO("Enter CWsEchoHandler::ProcMsg");
    ASSERT_RET_VALUE(pMsg, 1);
    if (pMsg->GetCli() && pMsg->GetMsg()) {
        LOG_INFO("Echo Recv :%s", pMsg->GetMsg()->payload);
        len_str lStr = sWsMsgParse->EncodeMsg(pMsg->GetMsg()->payload, pMsg->GetMsg()->payloadLength, WS_FRAME_TEXT);
        if (lStr.iLen > 0 && lStr.pStr) {
            pMsg->GetCli()->Send(lStr.pStr, lStr.iLen);
            DOFREE(lStr.pStr);
        }
    } else {
        LOG_ERR("No Msg or No Cli");
    }
    return 0;
}