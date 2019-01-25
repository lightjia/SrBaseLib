#include "WsInput.h"
#include "WsCli.h"
#include "WsMsg.h"
#include "WsHandlerMgr.h"
#include "WsMsgParse.h"
CWsInput::CWsInput(CWsCli* pCli) {
    miState = WS_STATE_NONE;
    mpCli = pCli;
    mpMsg = NULL;
    memset(&mstMsgCache, 0, sizeof(mstMsgCache));
}

CWsInput::~CWsInput() {
    if (mpMsg) {
        DOFREE(mpMsg->payload);
        DOFREE(mpMsg);
    }

    DOFREE(mstMsgCache.pData);
}

int CWsInput::ParseMsg(len_str& lStr) {
    int iRet = 1;
    ASSERT_RET_VALUE(lStr.iLen > 0 && lStr.pStr, 1);
    if (WS_STATE_NONE == miState) {
        if (str_start_with(lStr.pStr, GET)) {
            mstrProtocol = GET;
            if (strstr(lStr.pStr, WEBSOCKETKEY)) {
                mstrProtocol = WEBSOCKETKEY;
            }

            CWsMsg* pWsMsg = new CWsMsg(mstrProtocol, mpCli);
            std::string strHttp(lStr.pStr, lStr.iLen);
            pWsMsg->SetHttpMsg(strHttp);
            sWsHandlerMgr->ProcMsg(pWsMsg);
            iRet = 0;
        }
    }
    else {
        ASSERT_RET_VALUE(!mstrProtocol.empty(), 1);

        bool bFin = false;
        if (!mstMsgCache.pData) {
            mstMsgCache.pData = (char*)do_malloc(lStr.iLen * sizeof(char));
            memcpy(mstMsgCache.pData, lStr.pStr, lStr.iLen);
            mstMsgCache.iTotal = lStr.iLen;
            mstMsgCache.iUse = lStr.iLen;
        }
        else {
            if (mstMsgCache.iTotal >= mstMsgCache.iUse + lStr.iLen) {
                memcpy(mstMsgCache.pData + mstMsgCache.iUse, lStr.pStr, lStr.iLen);
                mstMsgCache.iUse += lStr.iLen;
            }
            else {
                char* psrcTmp = (char*)do_malloc((lStr.iLen + mstMsgCache.iUse) * sizeof(char));
                memcpy(psrcTmp, mstMsgCache.pData, mstMsgCache.iUse);
                memcpy(psrcTmp + mstMsgCache.iUse, lStr.pStr, lStr.iLen);
                DOFREE(mstMsgCache.pData);
                mstMsgCache.pData = psrcTmp;
                mstMsgCache.iUse += lStr.iLen;
                mstMsgCache.iTotal = mstMsgCache.iUse;
            }
        }

        do {
            iRet = sWsMsgParse->DecodeMsg(&mstMsgCache, &mpMsg);
            if (0 == iRet && mpMsg && mpMsg->complete) {
                CWsMsg* pWsMsg = new CWsMsg(mstrProtocol, mpCli);
                pWsMsg->SetMsg(mpMsg);
                sWsHandlerMgr->ProcMsg(pWsMsg);
                mpMsg = NULL;
            } else {
                LOG_ERR("Cache Use :%d", mstMsgCache.iUse);
            }
        } while (iRet == 0 && mstMsgCache.iUse > 0);
    }

    return iRet;
}

int CWsInput::ProcMsg() {
    LOG_INFO("Enter CWsInput::ProcMsg");
    for (;;) {
        len_str lstr;
        memset(&lstr, 0, sizeof(lstr));
        mcQueMsgMutex.Lock();
        if (!mqueMsg.empty()) {
            lstr = mqueMsg.front();
            mqueMsg.pop();
        }
        mcQueMsgMutex.UnLock();

        if (lstr.iLen <= 0 || !lstr.pStr) {
            break;
        }

        if (ParseMsg(lstr)) {
            LOG_ERR("Parse Msg Error");
            DOFREE(lstr.pStr);
            mpCli->Close();
            break;
        }

        DOFREE(lstr.pStr);
    }

    return 0;
}

int CWsInput::PushMsg(char* pMsg, ssize_t iLen) {
    ASSERT_RET_VALUE(pMsg && iLen > 0, 1);
    len_str lstr;
    lstr.iLen = iLen;
    lstr.pStr = (char*)do_malloc(iLen * sizeof(char));
    memcpy(lstr.pStr, pMsg, iLen);

    mcQueMsgMutex.Lock();
    mqueMsg.push(lstr);
    mcQueMsgMutex.UnLock();
    return 0;
}