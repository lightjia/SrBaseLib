#include "WsMsgParse.h"

CWsMsgParse::CWsMsgParse(){
}

CWsMsgParse::~CWsMsgParse(){
}

len_str CWsMsgParse::EncodeMsg(const char* pData, const size_t iLen, int iFrameType) {
    len_str lRet;
    BZERO(lRet);
    ASSERT_RET_VALUE(pData && iLen > 0, lRet);
    uint8_t payloadFieldExtraBytes = 0;
    if (iLen <= 125){
        payloadFieldExtraBytes = 0;
    } else if (iLen > 125 && iLen <= 65535){
        payloadFieldExtraBytes = 2;
    } else {
        payloadFieldExtraBytes = 8;
    }

    // header: 2字节, mask位设置为0(不加密), 则后面的masking key无须填写, 省略4字节  
    uint8_t frameHeaderSize = 2 + payloadFieldExtraBytes;
    uint8_t* pFrameHead = (uint8_t*)do_malloc(frameHeaderSize * sizeof(uint8_t));
    memset(pFrameHead, 0, frameHeaderSize);
    // fin位为1, 扩展位为0, 操作位为frameType  
    pFrameHead[0] = static_cast<uint8_t>(0x80 | iFrameType);

    // 填充数据长度  
    if (iLen <= 125){
        pFrameHead[1] = static_cast<uint8_t>(iLen);
    } else if (iLen > 125 && iLen <= 65535){
        pFrameHead[1] = 0x7e;
        uint16_t len = htons((uint16_t)iLen);
        memcpy(&pFrameHead[2], &len, payloadFieldExtraBytes);
    } else{
        pFrameHead[1] = 0x7f;
        char len[8];
        len[0] = (iLen >> 56) & 255;
        len[1] = (iLen >> 48) & 255;
        len[2] = (iLen >> 40) & 255;
        len[3] = (iLen >> 32) & 255;
        len[4] = (iLen >> 24) & 255;
        len[5] = (iLen >> 16) & 255;
        len[6] = (iLen >> 8) & 255;
        len[7] = (iLen)& 255;
        memcpy(&pFrameHead[2], len, payloadFieldExtraBytes);
    }

    // 填充数据  
    size_t frameSize = frameHeaderSize + iLen;
    lRet.pStr = (char*)do_malloc((frameSize)*sizeof(char));
    lRet.iLen = frameSize;
    memcpy(lRet.pStr, pFrameHead, frameHeaderSize);
    memcpy(lRet.pStr + frameHeaderSize, pData, iLen);

    DOFREE(pFrameHead);

    return lRet;
}

int CWsMsgParse::TryDecodeLen(char* pData, uint8_t& frameType, uint8_t& payloadFieldExtraBytes, size_t& payloadLength) {
    payloadFieldExtraBytes = 0;
    frameType = 0;
    payloadLength = 0;
    ASSERT_RET_VALUE(pData, 1);
    // mask位, 为1表示数据被加密  
    ASSERT_RET_VALUE((pData[1] & 0x80) == 0x80, 1);

    // 操作码  
    frameType = static_cast<uint8_t>(pData[0] & 0x0f);

    // 处理utf-8编码的文本帧  
    payloadLength = static_cast<uint16_t>(pData[1] & 0x7f);
    if (payloadLength == 0x7e) {
        payloadFieldExtraBytes = 2;
        payloadLength = ntohs(*(uint16_t*)(&pData[2]));
    } else if (payloadLength == 0x7f) {
        // 数据过长
        payloadLength = ntohl(*(u_long*)(&pData[2 + 4]));
        payloadFieldExtraBytes = 8;
    } 

    ASSERT_RET_VALUE(payloadLength > 0, 1);
    return 0;
}

int CWsMsgParse::DecodeMsg(tagWsMsgCache* pMsgCache, tagWsMsg** pWsMsg) {
    ASSERT_RET_VALUE(pMsgCache && pWsMsg && pMsgCache->iUse > 0, 1);
    if (pMsgCache->iCurFrameLen + pMsgCache->iCurFrameIndex > pMsgCache->iUse) {
        return 0;
    }

    uint8_t frameType = 0;
    uint8_t payloadFieldExtraBytes = 0;
    size_t payloadLength = 0;
    while (pMsgCache->iUse >= WS_MIN_MSG_EXPECT_LEN + pMsgCache->iCurFrameIndex) {
        // fin位: 为1表示已接收完整报文, 为0表示继续监听后续报文  
        bool bFin = true;
        if ((pMsgCache->pData[pMsgCache->iCurFrameIndex] & 0x80) != 0x80) {
            bFin = false;
        }

        ASSERT_RET_VALUE(!TryDecodeLen(pMsgCache->pData + pMsgCache->iCurFrameIndex, frameType, payloadFieldExtraBytes, payloadLength), 1);
        pMsgCache->iCurFrameLen = payloadFieldExtraBytes + WS_MIN_MSG_EXPECT_LEN + payloadLength;
        if (pMsgCache->iCurFrameLen + pMsgCache->iCurFrameIndex <= pMsgCache->iUse) {
            pMsgCache->iTotalFrameLen += payloadLength;
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
        size_t iOffset = 0;
        size_t iPayloadOffset = 0;
        while (pMsgCache->iCurFrameIndex > iOffset) {
            if (!(*pWsMsg)) {
                *pWsMsg = (tagWsMsg*)do_malloc(sizeof(tagWsMsg));
                (*pWsMsg)->payload = (char*)do_malloc((pMsgCache->iTotalFrameLen + 1)*sizeof(char));
                (*pWsMsg)->payload[pMsgCache->iTotalFrameLen] = '\0';
            }

            tagWsMsg* pTmpWsMsg = *pWsMsg;
            ASSERT_RET_VALUE(!TryDecodeLen(pMsgCache->pData + iOffset, frameType, payloadFieldExtraBytes, payloadLength), 1);
            if (!pTmpWsMsg->frameType) {
                pTmpWsMsg->frameType = frameType;
            }

            ASSERT_RET_VALUE(pTmpWsMsg->frameType == frameType || frameType == WS_FRAME_CONTINUATION, 1);
            // header: 2字节, masking key: 4字节  
            const char *maskingKey = &pMsgCache->pData[iOffset + 2 + payloadFieldExtraBytes];
            memcpy(pTmpWsMsg->payload + iPayloadOffset, &pMsgCache->pData[iOffset + 2 + payloadFieldExtraBytes + 4], payloadLength);
            for (int i = 0; i < payloadLength; i++) {
                pTmpWsMsg->payload[iPayloadOffset + i] = pTmpWsMsg->payload[iPayloadOffset + i] ^ maskingKey[(iPayloadOffset + i) % 4];
            }

            iOffset += payloadFieldExtraBytes + WS_MIN_MSG_EXPECT_LEN + payloadLength;
            iPayloadOffset += payloadLength;
            pTmpWsMsg->payloadLength += payloadLength;
        }

        pMsgCache->iComplete = 0;
        pMsgCache->iCurFrameIndex -= iOffset;
        pMsgCache->iUse -= iOffset;
        pMsgCache->iTotalFrameLen -= iPayloadOffset;
    }
   
    return 0;
}