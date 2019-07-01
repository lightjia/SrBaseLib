#include "WsCli.h"
#include "UvTaskPool.h"

CWsCli::CWsCli(){
    mbCloseFlag = false;
    mpInput = NULL;
    miActiveTime = time(NULL);
    LOG_INFO("new cli:%s", GetNetId().c_str());
}

CWsCli::~CWsCli(){
    DODELETE(mpInput);
    MemFree(mpTcpCli);
    LOG_INFO("Del cli:%s", GetNetId().c_str());
}

void CWsCli::OnTick() {
    time_t iNow = time(NULL);
    if (iNow > miActiveTime + WS_KEEP_ALIVE_SEC) {
        LOG_INFO("Cli:%p Timeout", this);
        Close();
    }
}

int CWsCli::OnRecv(ssize_t nRead, const uv_buf_t* pBuf) {
    LOG_INFO("Enter CWsCli::OnRecv:%d", nRead);
    if (nRead > 0 && mpInput) {
        miActiveTime = time(NULL);
        mpInput->PushMsg(pBuf->base, nRead);
		mcUvSem.Post();
    } else {
        LOG_ERR("No Handler");
    }

    return 0;
}

int CWsCli::OnConn(int iStatus) {
    return 0;
}

int CWsCli::OnClose() {
    mbCloseFlag = true;
    LOG_INFO("cli close:%s", GetNetId().c_str());
    return 0;
}

int CWsCli::OnSend(int iStatus) {
    LOG_INFO("Enter CWsCli::OnSend:%d iNeedSend:%I64u iTotalSend:%I64u", iStatus, GetNeedSendBytes(), GetTotalSendBytes());
    if (!iStatus) {
        miActiveTime = time(NULL);
    }
   
    return 0;
}

int CWsCli::OnInit() {
    mpInput = new CWsInput(this);
    LOG_INFO("Recv Buff Size:%d", mstUvBuf.iLen);
    return 0;
}

int CWsCli::TaskInit() {
    return 0;
}

int CWsCli::TaskExcute() {
	while(!mbCloseFlag){
		mcUvSem.Wait();
        mpInput->ProcMsg();
    }
    return 0;
}

int CWsCli::TaskQuit() {
    return 0;
}