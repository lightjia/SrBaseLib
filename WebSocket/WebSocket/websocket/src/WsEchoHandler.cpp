#include "WsEchoHandler.h"

CWsEchoHandler::CWsEchoHandler(){
}

CWsEchoHandler::~CWsEchoHandler(){
}

CMemBuffer* CWsEchoHandler::ProcMsg(CWsMsg* pMsg) {
    LOG_INFO("Enter CWsEchoHandler::ProcMsg");
	CMemBuffer* pRet = NULL;
    ASSERT_RET_VALUE(pMsg, pRet);
    if (pMsg->GetCli() && pMsg->GetMsg()) {
		pMsg->GetMsg()->pBuffer->AppendNul();
        LOG_INFO("Echo Recv MsgLen:%I64u Msg:%s", pMsg->GetMsg()->pBuffer->GetBuffLen(), (char*)pMsg->GetMsg()->pBuffer->GetBuffer());
		pRet = sWsMsgParse->EncodeMsg(pMsg->GetMsg()->pBuffer, WS_FRAME_TEXT);
    } else {
        LOG_ERR("No Msg or No Cli");
    }

    return pRet;
}