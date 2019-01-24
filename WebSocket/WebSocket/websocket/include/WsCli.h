#ifndef __CWSCLI__H_
#define __CWSCLI__H_
#include "Task.h"
#include "UvTcpCli.h"
#include "WsInput.h"
class CWsCli : public CUvTcpCli, public CTask{
public:
    CWsCli();
    ~CWsCli();

public:
    bool IsClosed() {  return mbCloseFlag; }
    void OnTick();
    CWsInput* GetInut() { return mpInput; }

public:
    int TaskInit();
    int TaskExcute();
    int TaskQuit();

protected:
    int OnRecv(ssize_t nRead, const uv_buf_t* pBuf);
    int OnConn(int iStatus);
    int OnClose();
    int OnSend(int iStatus);
    int OnInit();

private:
    bool mbCloseFlag;
    CWsInput* mpInput;
    CUvMutex mcInputMutex;
    time_t miActiveTime;
};

#endif