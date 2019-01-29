#include "WsMsgParse.h"

CWsMsgParse::CWsMsgParse(){
}

CWsMsgParse::~CWsMsgParse(){
}

len_str CWsMsgParse::EncodeMsg(const char* pData, const size_t iLen, int iFrameType) {
    len_str lRet;
    BZERO(lRet);
    ASSERT_RET_VALUE(pData && iLen > 0, lRet);
    size_t frameSize = iLen;

    //flags byte.
    frameSize += 1; 

    // Add the size bytes:
    if (iLen <= 125){
        frameSize += 1;
    } else if (iLen > 125 && iLen <= 65535){
        frameSize += 3;
    } else {
        frameSize += 9;
    }

    lRet.pStr = (char*)do_malloc((frameSize)*sizeof(char));
    lRet.iLen = frameSize;

    // fin位为1, 扩展位为0, 操作位为frameType  
    lRet.pStr[0] = 0x80 | iFrameType;

    // 填充数据长度  
    if (iLen <= 125){
        lRet.pStr[1] = (char)(iLen);
    } else if (iLen > 125 && iLen <= 65535){
        lRet.pStr[1] = 0x7e;

        u_short uNetLen = htons((u_short)iLen);
        memcpy(&lRet.pStr[2], &uNetLen, sizeof(uNetLen));
    } else{
        lRet.pStr[1] = 0x7f;
        u_long lNetLen = htonl((u_long)iLen);
        memcpy(&lRet.pStr[2], &lNetLen, sizeof(lNetLen));
    }

    // 填充数据  
    int iOffset = (int)(frameSize - iLen);
    memcpy(lRet.pStr + iOffset, pData, iLen);

    return lRet;
}

int CWsMsgParse::TryDecodeLen(char* pData, tagWsMsgFrame& stFrame) {
    BZERO(stFrame);
    stFrame.expectsize = WS_MIN_MSG_EXPECT_LEN;
    ASSERT_RET_VALUE(pData, 1);
    // 操作码  
    stFrame.frameType = static_cast<uint8_t>(pData[0] & 0x0f);

    // 处理utf-8编码的文本帧  
    stFrame.payloadLength = static_cast<uint16_t>(pData[1] & 0x7f);
    stFrame.payloadOffset = 2;

    if (stFrame.payloadLength == 0x7e) {
        //Add 2 bytes for size:
        stFrame.expectsize += 2;
        stFrame.payloadOffset = 4;
        stFrame.payloadLength = ntohs(*(uint16_t*)(&pData[2]));
    } else if (stFrame.payloadLength == 0x7f) {
        // Add 8 bytes for size
        stFrame.expectsize += 8;
        stFrame.payloadLength = ntohl(*(u_long*)(&pData[2 + 4]));
        stFrame.payloadOffset = 10;
    } 

    ASSERT_RET_VALUE(stFrame.payloadLength > 0, 1);
    stFrame.expectsize += stFrame.payloadLength;

    // mask位, 为1表示数据被加密  
    if (pData[1] & 0x80) {
        stFrame.mask = 1;
        stFrame.payloadOffset += 4;
        stFrame.expectsize += 4;
    }
    return 0;
}

int CWsMsgParse::DecodeMsg(tagWsMsgCache* pMsgCache, tagWsMsg** pWsMsg) {
    ASSERT_RET_VALUE(pMsgCache && pWsMsg && pMsgCache->iUse > 0, 1);
    LOG_INFO("Enter CWsMsgParse::DecodeMsg iCurFrameLen:%d iCurFrameIndex:%d iUse:%d", pMsgCache->iCurFrameLen, pMsgCache->iCurFrameIndex, pMsgCache->iUse);
    if (pMsgCache->iCurFrameLen + pMsgCache->iCurFrameIndex > pMsgCache->iUse) {
        return 0;
    }

    while (pMsgCache->iUse >= WS_MIN_MSG_EXPECT_LEN + pMsgCache->iCurFrameIndex) {
        // fin位: 为1表示已接收完整报文, 为0表示继续监听后续报文  
        bool bFin = true;
        if ((pMsgCache->pData[pMsgCache->iCurFrameIndex] & 0x80) != 0x80) {
            bFin = false;
        }

        tagWsMsgFrame stFrame;
        ASSERT_RET_VALUE(!TryDecodeLen(pMsgCache->pData + pMsgCache->iCurFrameIndex, stFrame), 1);
        pMsgCache->iCurFrameLen = stFrame.expectsize;
        if (pMsgCache->iCurFrameLen + pMsgCache->iCurFrameIndex <= pMsgCache->iUse) {
            pMsgCache->iTotalFrameLen += stFrame.payloadLength;
            pMsgCache->iCurFrameIndex += pMsgCache->iCurFrameLen;
            if (bFin) {
                pMsgCache->iComplete = 1;
                break;
            }
        } else {
            break;
        }
    }

    if (pMsgCache->iComplete) {
        LOG_INFO("Recv An Frame");
        size_t iOffset = 0;
        size_t iPayloadOffset = 0;
        while (pMsgCache->iCurFrameIndex > iOffset) {
            if (!(*pWsMsg)) {
                *pWsMsg = (tagWsMsg*)do_malloc(sizeof(tagWsMsg));
                (*pWsMsg)->payload = (char*)do_malloc((pMsgCache->iTotalFrameLen + 1)*sizeof(char));
                (*pWsMsg)->payload[pMsgCache->iTotalFrameLen] = '\0';
            }

            tagWsMsg* pTmpWsMsg = *pWsMsg;
            tagWsMsgFrame stFrame;
            ASSERT_RET_VALUE(!TryDecodeLen(pMsgCache->pData + iOffset, stFrame), 1);
            if (!pTmpWsMsg->frameType) {
                pTmpWsMsg->frameType = stFrame.frameType;
            }

            ASSERT_RET_VALUE(pTmpWsMsg->frameType == stFrame.frameType || stFrame.frameType == WS_FRAME_CONTINUATION, 1);
            memcpy(pTmpWsMsg->payload + iPayloadOffset, &pMsgCache->pData[iOffset + stFrame.payloadOffset], stFrame.payloadLength);
            if (stFrame.mask && stFrame.payloadOffset > 4) {
                // header: 2字节, masking key: 4字节  
                const char *maskingKey = &pMsgCache->pData[stFrame.payloadOffset - 4];
                for (int i = 0; i < stFrame.payloadLength; i++) {
                    pTmpWsMsg->payload[iPayloadOffset + i] = pTmpWsMsg->payload[iPayloadOffset + i] ^ maskingKey[i % 4];
                }
            }

            iOffset += stFrame.expectsize;
            iPayloadOffset += stFrame.payloadLength;
            pTmpWsMsg->payloadLength += stFrame.payloadLength;
        }

        ASSERT_RET_VALUE(*pWsMsg && (*pWsMsg)->payloadLength == pMsgCache->iTotalFrameLen, 1);
        pMsgCache->iComplete = 0;
        pMsgCache->iCurFrameIndex -= iOffset;
        pMsgCache->iUse -= iOffset;
        pMsgCache->iTotalFrameLen -= iPayloadOffset;
        pMsgCache->iCurFrameLen = 0;
    }
   
    LOG_INFO("Leave CWsMsgParse::DecodeMsg iCurFrameLen:%d iCurFrameIndex:%d iUse:%d", pMsgCache->iCurFrameLen, pMsgCache->iCurFrameIndex, pMsgCache->iUse);
    return 0;
}