#include "WsMsgParse.h"

CWsMsgParse::CWsMsgParse(){
}

CWsMsgParse::~CWsMsgParse(){
}

CMemBuffer* CWsMsgParse::EncodeMsg(CMemBuffer* pMsg, int iFrameType) {
	ASSERT_RET_VALUE(pMsg, NULL);
	const char* pData = (char*)pMsg->GetBuffer();
	const size_t iLen = pMsg->GetBuffLen();
    ASSERT_RET_VALUE(pData && iLen > 0, NULL);
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

	CMemBuffer* pRet = new CMemBuffer();
	pRet->AllocBuffer(frameSize);
	pRet->SetBuffLen(frameSize);
	char* pRetMsg = (char*)pRet->GetBuffer();
    // fin位为1, 扩展位为0, 操作位为frameType  
	pRetMsg[0] = 0x80 | iFrameType;
	
    // 填充数据长度  
    if (iLen <= 125){
		pRetMsg[1] = (char)(iLen);
    } else if (iLen > 125 && iLen <= 65535){
		pRetMsg[1] = 0x7e;
		char len[2];
		len[0] = (iLen >> 8) & 255;
		len[1] = (iLen)& 255;
        memcpy(&pRetMsg[2], len, 2);
    } else{
		pRetMsg[1] = 0x7f;
		char len[8];
		len[0] = (iLen >> 56) & 255;
		len[1] = (iLen >> 48) & 255;
		len[2] = (iLen >> 40) & 255;
		len[3] = (iLen >> 32) & 255;
		len[4] = (iLen >> 24) & 255;
		len[5] = (iLen >> 16) & 255;
		len[6] = (iLen >> 8) & 255;
		len[7] = (iLen)& 255;
		memcpy(&pRetMsg[2], len, 8);
		/* u_long lNetLen = htonl((u_long)iLen);
		 memcpy(&lRet.pStr[2], &lNetLen, sizeof(lNetLen));*/
    }

    // 填充数据  
    size_t iOffset = frameSize - iLen;
    memcpy(pRetMsg + iOffset, pData, iLen);

    return pRet;
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
    ASSERT_RET_VALUE(pMsgCache && pWsMsg && pMsgCache->pBuffer  && pMsgCache->pBuffer->GetBuffLen() > 0, 1);
    LOG_INFO("Enter CWsMsgParse::DecodeMsg iCurFrameLen:%I64u iCurFrameIndex:%I64u iUse:%I64u", pMsgCache->iCurFrameLen, pMsgCache->iCurFrameIndex, pMsgCache->pBuffer->GetBuffLen());
    if (pMsgCache->iCurFrameLen + pMsgCache->iCurFrameIndex > pMsgCache->pBuffer->GetBuffLen()) {
        return 0;
    }

    while (pMsgCache->pBuffer->GetBuffLen() >= WS_MIN_MSG_EXPECT_LEN + pMsgCache->iCurFrameIndex) {
        // fin位: 为1表示已接收完整报文, 为0表示继续监听后续报文  
		char* pMsgPkg = (char*)pMsgCache->pBuffer->GetBuffer();
        bool bFin = true;
        if ((pMsgPkg[pMsgCache->iCurFrameIndex] & 0x80) != 0x80) {
            bFin = false;
        }

        tagWsMsgFrame stFrame;
        ASSERT_RET_VALUE(!TryDecodeLen(pMsgPkg + pMsgCache->iCurFrameIndex, stFrame), 1);
        pMsgCache->iCurFrameLen = stFrame.expectsize;
        if (pMsgCache->iCurFrameLen + pMsgCache->iCurFrameIndex <= pMsgCache->pBuffer->GetBuffLen()) {
            pMsgCache->iTotalFrameLen += stFrame.payloadLength;
            pMsgCache->iCurFrameIndex += pMsgCache->iCurFrameLen;
			pMsgCache->iCurFrameLen = 0;
            if (bFin) {
                pMsgCache->iComplete = 1;
                break;
            }
        } else {
            break;
        }
    }

    if (pMsgCache->iComplete) {
        LOG_INFO("Recv An Frame Len:%I64u", pMsgCache->iTotalFrameLen);
        size_t iOffset = 0;
        size_t iPayloadOffset = 0;
		char* pMsgPkg = (char*)pMsgCache->pBuffer->GetBuffer();
        while (pMsgCache->iCurFrameIndex > iOffset && iPayloadOffset <= pMsgCache->iTotalFrameLen) {
            if (!(*pWsMsg)) {
                *pWsMsg = new tagWsMsg();
				(*pWsMsg)->pBuffer = new CMemBuffer();
            }

            tagWsMsg* pTmpWsMsg = *pWsMsg;
            tagWsMsgFrame stFrame;
            ASSERT_RET_VALUE(!TryDecodeLen(pMsgPkg + iOffset, stFrame), 1);
            if (!pTmpWsMsg->frameType) {
                pTmpWsMsg->frameType = stFrame.frameType;
            }

            ASSERT_RET_VALUE(pTmpWsMsg->frameType == stFrame.frameType || stFrame.frameType == WS_FRAME_CONTINUATION, 1);
			pTmpWsMsg->pBuffer->Append(&pMsgPkg[iOffset + stFrame.payloadOffset], stFrame.payloadLength);
            if (stFrame.mask && stFrame.payloadOffset > 4) {
                // header: 2字节, masking key: 4字节  
                const char *maskingKey = &pMsgPkg[iOffset + stFrame.payloadOffset - 4];
				char* pWsMsg = (char*)pTmpWsMsg->pBuffer->GetBuffer();
                for (size_t i = 0; i < stFrame.payloadLength; i++) {
					pWsMsg[iPayloadOffset + i] = pWsMsg[iPayloadOffset + i] ^ maskingKey[i % 4];
                }
            }

            iOffset += stFrame.expectsize;
            iPayloadOffset += stFrame.payloadLength;
        }

        ASSERT_RET_VALUE(*pWsMsg && (*pWsMsg)->pBuffer->GetBuffLen() == pMsgCache->iTotalFrameLen, 1);
        pMsgCache->iComplete = 0;
        pMsgCache->iCurFrameIndex -= iOffset;
		size_t iRemain = pMsgCache->pBuffer->GetBuffLen() - iOffset;
		if (iRemain > 0) {
			memmove(pMsgPkg, pMsgPkg + iOffset, iRemain);
		}
		pMsgCache->pBuffer->SetBuffLen(iRemain);
        pMsgCache->iTotalFrameLen -= iPayloadOffset;
        pMsgCache->iCurFrameLen = 0;
    }
   
    LOG_INFO("Leave CWsMsgParse::DecodeMsg iCurFrameLen:%I64u iCurFrameIndex:%I64u iUse:%I64u", pMsgCache->iCurFrameLen, pMsgCache->iCurFrameIndex, pMsgCache->pBuffer->GetBuffLen());
    return 0;
}