#include "WsGetHandler.h"

CWsGetHandler::CWsGetHandler(){
}

CWsGetHandler::~CWsGetHandler(){
}

CMemBuffer* CWsGetHandler::ProcMsg(CWsMsg* pMsg) {
    LOG_INFO("Enter CWsGetHandler::ProcMsg");
	CMemBuffer* pRet = NULL;
    ASSERT_RET_VALUE(pMsg, pRet);
	pRet = new CMemBuffer();
#define DEFAULT_HTTP "index.html"
    std::string strPwd = get_app_path();
    strPwd += DEFAULT_HTTP;
    const long lLen = get_file_len(strPwd.c_str(), "r");
    if (lLen > 0) {
		char szTmp[200];
		snprintf(szTmp, 200, "HTTP/1.1 200 OK\r\nContent-Type: html\r\nContent-Length: %ld\r\n\r\n", lLen);
		pRet->AllocBuffer(lLen + 200);
		pRet->Append(szTmp, strlen(szTmp));
        pRet->SetBuffLen(pRet->GetBuffLen() + file_read(strPwd.c_str(), "r", (unsigned char*)(pRet->GetBuffer(pRet->GetBuffLen())), lLen, 0));
    } else {
        LOG_ERR("html:%s not exist", strPwd.c_str());
    }

    return pRet;
}