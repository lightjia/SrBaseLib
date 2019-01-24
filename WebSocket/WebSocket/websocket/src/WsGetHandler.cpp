#include "WsGetHandler.h"

len_str CWsGetHandler::mlHtml;
CWsGetHandler::CWsGetHandler(){
    mbLoadHtml = false;
}

CWsGetHandler::~CWsGetHandler(){
}

int CWsGetHandler::ProcMsg(CWsMsg* pMsg) {
    ASSERT_RET_VALUE(pMsg, 1);
    if (!mbLoadHtml) {
        mcHtmlMux.Lock();
        if (!mbLoadHtml) {
#define DEFAULT_HTTP "index.html"
            std::string strPwd = get_app_path();
            strPwd += DEFAULT_HTTP;
            const long lLen = get_file_len(strPwd.c_str(), "r");
            if (lLen > 0) {
                mlHtml.pStr = (char*)do_malloc(lLen * sizeof(char));
                mlHtml.iLen = file_read(strPwd.c_str(), "r", (unsigned char*)mlHtml.pStr, lLen, 0);
                mbLoadHtml = true;
            }
            else {
                LOG_ERR("html:%s not exist", strPwd.c_str());
            }
        }
        mcHtmlMux.UnLock();
    }

    ASSERT_RET_VALUE(mbLoadHtml && mlHtml.iLen > 0, 1);
    char szTmp[200];
    snprintf(szTmp, 200, "HTTP/1.1 200 OK\r\nContent-Type: html\r\nContent-Length: %d\r\n\r\n", (int)mlHtml.iLen);
    ssize_t iDataLen = strlen(szTmp) + mlHtml.iLen;
    char* pData = (char*)do_malloc(iDataLen);
    size_t iOffset = 0;
    memcpy(pData, szTmp, strlen(szTmp));
    iOffset += strlen(szTmp);
    memcpy(pData + iOffset, mlHtml.pStr, mlHtml.iLen);

    if (pMsg->GetCli()) {
        pMsg->GetCli()->Send(pData, iDataLen);
    }
    return 0;
}