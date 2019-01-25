#include "WsUpgradeHandler.h"
#include "openssl/sha.h"
#include "openssl/pem.h"
#include "openssl/bio.h"
#include "openssl/evp.h"
#include <sstream>

char * Base64Encode(const char * input, int length, bool with_new_line = false){
    ASSERT_RET_VALUE(input && length > 0, NULL);
    BIO *b64 = BIO_new(BIO_f_base64());
    if (!with_new_line) {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    }

    BIO *bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, length);
    BIO_flush(b64);
    BUF_MEM * bptr = NULL;
    BIO_get_mem_ptr(b64, &bptr);

    char * buff = (char *)do_malloc((bptr->length + 1) * sizeof(char));
    memcpy(buff, bptr->data, bptr->length);
    buff[bptr->length] = 0;

    BIO_free_all(b64);
    return buff;
}

char* Base64Decode(char * input, int length, bool with_new_line){
    char * buffer = (char *)do_malloc(length * sizeof(char));
    BIO *b64 = BIO_new(BIO_f_base64());
    if (!with_new_line) {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    }
    BIO * bmem = BIO_new_mem_buf(input, length);
    bmem = BIO_push(b64, bmem);
    BIO_read(bmem, buffer, length);

    BIO_free_all(bmem);

    return buffer;
}

CWsUpgradeHandler::CWsUpgradeHandler(){
}

CWsUpgradeHandler::~CWsUpgradeHandler(){
}

len_str CWsUpgradeHandler::ProcMsg(CWsMsg* pMsg) {
    LOG_INFO("Enter CWsUpgradeHandler::ProcMsg");
    len_str lRet;
    memset(&lRet, 0, sizeof(lRet));
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
    uint8_t pHash[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char*)websocketKey.c_str(), strlen(websocketKey.c_str()), (unsigned char*)pHash);
    char* pEncode = Base64Encode((char*)pHash, (int)SHA_DIGEST_LENGTH);
    ASSERT_RET_VALUE(pEncode, lRet);
    iOffset += snprintf(szTmp + iOffset, WS_UPGRADE_RESPONSE_LEN - iOffset, "%s", pEncode);
    DOFREE(pEncode);
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