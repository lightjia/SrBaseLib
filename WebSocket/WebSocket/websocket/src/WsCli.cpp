#include "WsCli.h"
#include "UvTaskPool.h"

CWsCli::CWsCli(){
    mbCloseFlag = false;
    mpInput = NULL;
    miActiveTime = time(NULL);
    LOG_INFO("new cli:%p", this);
}

CWsCli::~CWsCli(){
    DODELETE(mpInput);
    DOFREE(mpTcpCli);
    LOG_INFO("Del cli:%p", this);
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
        sUvTaskPool->PushTask((CTask*)this);
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
    LOG_INFO("cli close:%p", this);
    return 0;
}

int CWsCli::OnSend(int iStatus) {
    LOG_INFO("CWsCli::OnSend:%d", iStatus);
    miActiveTime = time(NULL);
    return 0;
}

int CWsCli::OnInit() {
    mpInput = new CWsInput(this);
    return 0;
}

int CWsCli::TaskInit() {
    return 0;
}

int CWsCli::TaskExcute() {
    if (!mcInputMutex.TryLock() && mpInput) {
        mpInput->ProcMsg();
        mcInputMutex.UnLock();
    }
    return 0;
}

int CWsCli::TaskQuit() {
    return 0;
}