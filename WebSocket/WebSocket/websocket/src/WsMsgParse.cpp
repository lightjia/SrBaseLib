#include "WsMsgParse.h"

CWsMsgParse::CWsMsgParse(){
}

CWsMsgParse::~CWsMsgParse(){
}

len_str CWsMsgParse::EncodeMsg(const char* pData, const size_t iLen, int iFrameType) {
    len_str lRet;
    memset(&lRet, 0, sizeof(lRet));
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

int CWsMsgParse::DecodeMsg(tagWsMsgCache* pMsgCache, tagWsMsg** pWsMsg) {
    ASSERT_RET_VALUE(pMsgCache && pWsMsg, 2);
    if (!(*pWsMsg)) {
        *pWsMsg = (tagWsMsg*)do_malloc(sizeof(tagWsMsg));
    }

    tagWsMsg* pTmpWsMsg = *pWsMsg;
    if (pTmpWsMsg->payloadLength == 0 && pMsgCache->iUse >= WS_MIN_MSG_EXPECT_LEN) {
        // 检查扩展位并忽略  
        ASSERT_RET_VALUE((pMsgCache->pData[0] & 0x40) == 0x0, 2);
        // fin位: 为1表示已接收完整报文, 为0表示继续监听后续报文  
        if ((pMsgCache->pData[0] & 0x80) != 0x80) {
            return 1;
        }

        // mask位, 为1表示数据被加密  
        ASSERT_RET_VALUE((pMsgCache->pData[1] & 0x80) == 0x80, 2);

        // 操作码  
        pTmpWsMsg->frameType = static_cast<uint8_t>(pMsgCache->pData[0] & 0x0f);
        // 处理utf-8编码的文本帧  
        pTmpWsMsg->payloadLength = static_cast<uint16_t>(pMsgCache->pData[1] & 0x7f);
        if (pTmpWsMsg->payloadLength == 0x7e) {
            uint16_t payloadLength16b = 0;
            pTmpWsMsg->payloadFieldExtraBytes = 2;
            memcpy(&payloadLength16b, &pMsgCache->pData[2], pTmpWsMsg->payloadFieldExtraBytes);
            pTmpWsMsg->payloadLength = ntohs(payloadLength16b);
        }
        else if (pTmpWsMsg->payloadLength == 0x7f) {
            // 数据过长
            pTmpWsMsg->payloadLength = ntohl(*(u_long*)(&pMsgCache->pData[2]));
            pTmpWsMsg->payloadFieldExtraBytes = 8;
        }
    }

    size_t iFrameLen = pTmpWsMsg->payloadFieldExtraBytes + WS_MIN_MSG_EXPECT_LEN + pTmpWsMsg->payloadLength;
    if (pMsgCache->iUse >= iFrameLen) {
        // header: 2字节, masking key: 4字节  
        const char *maskingKey = &pMsgCache->pData[2 + pTmpWsMsg->payloadFieldExtraBytes];
        pTmpWsMsg->payload = (char*)do_malloc((pTmpWsMsg->payloadLength + 1)*sizeof(char));
        memcpy(pTmpWsMsg->payload, &pMsgCache->pData[2 + pTmpWsMsg->payloadFieldExtraBytes + 4], pTmpWsMsg->payloadLength);
        for (int i = 0; i < pTmpWsMsg->payloadLength; i++) {
            pTmpWsMsg->payload[i] = pTmpWsMsg->payload[i] ^ maskingKey[i % 4];
        }
        pTmpWsMsg->payload[pTmpWsMsg->payloadLength] = '\0';


        if (pMsgCache->iUse == iFrameLen) {
            pMsgCache->iUse = 0;
            return 0;
        }


        memmove(pMsgCache->pData, pMsgCache->pData + iFrameLen, pMsgCache->iUse - iFrameLen);
        pMsgCache->iUse -= iFrameLen;
    }
    else {
        return 1;
    }

    return 0;
}