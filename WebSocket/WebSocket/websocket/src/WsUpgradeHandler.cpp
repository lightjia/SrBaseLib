#include "WsUpgradeHandler.h"
#include "OpensslBase64.h"
#include "OpensslSha.h"
#include <sstream>

CWsUpgradeHandler::CWsUpgradeHandler(){
}

CWsUpgradeHandler::~CWsUpgradeHandler(){
}

len_str CWsUpgradeHandler::ProcMsg(CWsMsg* pMsg) {
    LOG_INFO("Enter CWsUpgradeHandler::ProcMsg");
    len_str lRet;
    BZERO(lRet);
    ASSERT_RET_VALUE(pMsg, lRet);
    std::istringstream stream(pMsg->GetHttpMsg().c_str());
    std::string header;
    std::string::size_type pos = 0;
    std::string websocketKey;
    std::string websocketProtocol;
    while (std::getline(stream, header) && header != "\r"){
        header.erase(header.end() - 1);
        pos = header.find(": ", 0);
        if (pos != std::string::npos){
            std::string key = header.substr(0, pos);
            std::string value = header.substr(pos + 2);
            if (key == WEBSOCKETKEY){
                websocketKey = value;
            } else  if (key == WEBSOCKETPROTOCOL) {
                websocketProtocol = value;
            }
        }
    }

    ASSERT_RET_VALUE(!websocketKey.empty(), lRet);

    // 填充http响应头信息 
#define WS_UPGRADE_RESPONSE_LEN 600
    char szTmp[WS_UPGRADE_RESPONSE_LEN];
    int iOffset = 0;
    iOffset += snprintf(szTmp, WS_UPGRADE_RESPONSE_LEN, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: upgrade\r\nSec-WebSocket-Accept: ");
    websocketKey += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    len_str lSha = sOpensslSha->Sha1((const unsigned char*)websocketKey.c_str(), (int)strlen(websocketKey.c_str()));
    ASSERT_RET_VALUE(lSha.pStr && lSha.iLen > 0, lRet);
    len_str lEncode = sOpensslBase64->Base64Encode((char*)lSha.pStr, (int)lSha.iLen);
    DOFREE(lSha.pStr);
    ASSERT_RET_VALUE(lEncode.pStr && lEncode.iLen > 0, lRet);
    iOffset += snprintf(szTmp + iOffset, WS_UPGRADE_RESPONSE_LEN - iOffset, "%s", lEncode.pStr);
    DOFREE(lEncode.pStr);
    if (!websocketProtocol.empty()){
        iOffset += snprintf(szTmp + iOffset, WS_UPGRADE_RESPONSE_LEN - iOffset, "\r\nSec-WebSocket-Protocol: %s", websocketProtocol.c_str());
    } else {
        websocketProtocol = BLANK;
    }

    iOffset += snprintf(szTmp + iOffset, WS_UPGRADE_RESPONSE_LEN - iOffset, "\r\n\r\n");
    lRet.pStr = (char*)do_malloc(iOffset*sizeof(char));
    lRet.iLen = iOffset;
    memcpy(lRet.pStr, szTmp, iOffset);
    if (pMsg->GetCli() && pMsg->GetCli()->GetInut()) {
        pMsg->GetCli()->GetInut()->SetProtocol(websocketProtocol);
        pMsg->GetCli()->GetInut()->SetState(WS_STATE_CONNECT);
    }

    return lRet;
}