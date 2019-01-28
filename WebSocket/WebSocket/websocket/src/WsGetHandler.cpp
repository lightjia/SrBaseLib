#include "WsGetHandler.h"

CWsGetHandler::CWsGetHandler(){
}

CWsGetHandler::~CWsGetHandler(){
}

len_str CWsGetHandler::ProcMsg(CWsMsg* pMsg) {
    LOG_INFO("Enter CWsGetHandler::ProcMsg");
    len_str lHtml;
    BZERO(lHtml);
    ASSERT_RET_VALUE(pMsg, lHtml);
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

    ASSERT_RET_VALUE(lHtml.iLen > 0 && lHtml.pStr, lHtml);
    char szTmp[200];
    snprintf(szTmp, 200, "HTTP/1.1 200 OK\r\nContent-Type: html\r\nContent-Length: %d\r\n\r\n", (int)lHtml.iLen);
    ssize_t iDataLen = strlen(szTmp) + lHtml.iLen;
    char* pData = (char*)do_malloc(iDataLen);
    size_t iOffset = 0;
    memcpy(pData, szTmp, strlen(szTmp));
    iOffset += strlen(szTmp);
    memcpy(pData + iOffset, lHtml.pStr, lHtml.iLen);

    DOFREE(lHtml.pStr);
    lHtml.pStr = pData;
    lHtml.iLen = iDataLen;

    return lHtml;
}