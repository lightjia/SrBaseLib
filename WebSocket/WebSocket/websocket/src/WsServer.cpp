#include "WsServer.h"
#include "UvTaskPool.h"
CWsServer::CWsServer(){
}

CWsServer::~CWsServer(){
}

int CWsServer::OnInit() {
    mpTcpSvr = &mstSvrTcp;
    Listen();
    return StartBaseTimer(1000, 1000);
}

void CWsServer::OnBaseTimer() {
    CWsCli* pCli = NULL;
    //LOG_INFO("Enter CWsServer::OnBaseTimer");
    mcvecClisMutex.Lock();
    std::vector<CWsCli*>::iterator iter = mvecClis.begin();
    while (iter != mvecClis.end()) {
        CWsCli* pCli = *iter;
        if (pCli && pCli->IsClosed()) {
            UNREF(pCli);
            iter = mvecClis.erase(iter);
            continue;
        }

        pCli->OnTick();
        ++iter;
    }
    mcvecClisMutex.UnLock();
}

int CWsServer::AddCli(uv_tcp_t* pUvTcp) {
    ASSERT_RET_VALUE(pUvTcp, 1);
    CWsCli* pCli = new CWsCli();
    if (pCli) {
        pCli->SetUvLoop(GetUvLoop());
        pCli->SetTcpCli(pUvTcp);
        pCli->Init();
        REF(pCli);
        LOG_INFO("CWsServer::AddCli IP:%s Port:%d", pCli->GetIp().c_str(), pCli->GetPort());
        sUvTaskPool->PushTask((CTask*)pCli);
        mcvecClisMutex.Lock();
        mvecClis.push_back(pCli);
        mcvecClisMutex.UnLock();
    }
   
    return 0;
}

int CWsServer::OnAccept(uv_tcp_t* pUvTcp) {
    LOG_INFO("Enter CWsServer::OnAccept:%p", pUvTcp);
    ASSERT_RET_VALUE(pUvTcp, 1);
    return AddCli(pUvTcp);
}

int CWsServer::BroadCast(char* pData, ssize_t iLen) {
    CWsCli* pCli = NULL;
    mcvecClisMutex.Lock();
    for (std::vector<CWsCli*>::iterator iter = mvecClis.begin(); iter != mvecClis.end(); ++iter) {
        pCli = *iter;
        if (NULL != pCli && !pCli->IsClosed()) {
            pCli->Send(pData, iLen);
        }
    }
    mcvecClisMutex.UnLock();
    return 0;
}