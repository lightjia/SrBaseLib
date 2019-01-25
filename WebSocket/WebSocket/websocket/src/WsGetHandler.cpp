#include "WsGetHandler.h"

CWsGetHandler::CWsGetHandler(){
}

CWsGetHandler::~CWsGetHandler(){
}

int CWsGetHandler::ProcMsg(CWsMsg* pMsg) {
    LOG_INFO("Enter CWsGetHandler::ProcMsg");
    ASSERT_RET_VALUE(pMsg, 1);
    len_str lHtml;
    memset(&lHtml, 0, sizeof(lHtml));
#define DEFAULT_HTTP "index.html"
    std::string strPwd = get_app_path();
    strPwd += DEFAULT_HTTP;
    const long lLen = get_file_len(strPwd.c_str(), "r");
    if (lLen > 0) {
        lHtml.pStr = (char*)do_malloc(lLen * sizeof(char));
        lHtml.iLen = file_read(strPwd.c_str(), "r", (unsigned char*)lHtml.pStr, lLen, 0);
    } else {
        LOG_ERR("html:%s not exist", strPwd.c_str());
    }

    ASSERT_RET_VALUE(lHtml.iLen > 0, 1);
    char szTmp[200];
    snprintf(szTmp, 200, "HTTP/1.1 200 OK\r\nContent-Type: html\r\nContent-Length: %d\r\n\r\n", (int)lHtml.iLen);
    ssize_t iDataLen = strlen(szTmp) + lHtml.iLen;
    char* pData = (char*)do_malloc(iDataLen);
    size_t iOffset = 0;
    memcpy(pData, szTmp, strlen(szTmp));
    iOffset += strlen(szTmp);
    memcpy(pData + iOffset, lHtml.pStr, lHtml.iLen);

    if (pMsg->GetCli()) {
        pMsg->GetCli()->Send(pData, iDataLen);
    }

    return 0;
}