#include "WsInput.h"
#include "WsCli.h"
#include "WsMsg.h"
#include "WsHandlerMgr.h"
#include "WsMsgParse.h"
CWsInput::CWsInput(CWsCli* pCli) {
    miState = WS_STATE_NONE;
    mpCli = pCli;
    mpMsg = NULL;
	mpRecvBuffer = new CMemBuffer();
	mstMsgCache.pBuffer = new CMemBuffer();
}

CWsInput::~CWsInput() {
    if (mpMsg) {
		DODELETE(mpMsg->pBuffer);
        DODELETE(mpMsg);
    }

    DODELETE(mstMsgCache.pBuffer);
    mcRecvBufferMutex.Lock();
	DODELETE(mpRecvBuffer);
	mcRecvBufferMutex.UnLock();
}

int CWsInput::ParseMsg() {
	if (mstMsgCache.pBuffer->GetBuffLen() <= 0) {
		return 0;
	}

	LOG_INFO("Enter CWsInput::ParseMsg:%I64u", mstMsgCache.pBuffer->GetBuffLen());
    if (WS_STATE_NONE == miState) {
		mstMsgCache.pBuffer->AppendNul();
		char* pMsg = (char*)mstMsgCache.pBuffer->GetBuffer();
        if (str_start_with(pMsg, GET)) {
            mstrProtocol = GET;
            if (strstr(pMsg, WEBSOCKETKEY)) {
                mstrProtocol = WEBSOCKETKEY;
            }

            CWsMsg* pWsMsg = new CWsMsg(mstrProtocol, mpCli);
            std::string strHttp(pMsg, mstMsgCache.pBuffer->GetBuffLen());
            pWsMsg->SetHttpMsg(strHttp);
            sWsHandlerMgr->ProcMsg(pWsMsg);
			mstMsgCache.pBuffer->SetBuffLen(0);
        }
    } else {
        ASSERT_RET_VALUE(!mstrProtocol.empty(), 1);
		while (sWsMsgParse->DecodeMsg(&mstMsgCache, &mpMsg)) {
			if (mpMsg) {
				CWsMsg* pWsMsg = new CWsMsg(mstrProtocol, mpCli);
				pWsMsg->SetMsg(mpMsg);
				sWsHandlerMgr->ProcMsg(pWsMsg);
				mpMsg = NULL;
			}
		}
    }

    return 0;
}

int CWsInput::ProcMsg() {
	mcRecvBufferMutex.Lock();
	if (mpRecvBuffer->GetBuffLen() > 0) {
		mstMsgCache.pBuffer->Append(mpRecvBuffer->GetBuffer(), mpRecvBuffer->GetBuffLen());
		mpRecvBuffer->SetBuffLen(0);
	}
	mcRecvBufferMutex.UnLock();

	return ParseMsg();
}

int CWsInput::PushMsg(char* pMsg, ssize_t iLen) {
    ASSERT_RET_VALUE(pMsg && iLen > 0, 1);
    mcRecvBufferMutex.Lock();
	mpRecvBuffer->Append(pMsg, iLen);
	mcRecvBufferMutex.UnLock();
    return 0;
}