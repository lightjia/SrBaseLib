#ifndef __CWSSERVER__H_
#define __CWSSERVER__H_
#include "UvTcpSvr.h"
#include "WsCli.h"
#include "singleton.h"
#include <vector>
class CWsServer : public CUvTcpSvr, public CSingleton<CWsServer> {
    SINGLE_CLASS_INITIAL(CWsServer);
public:
    ~CWsServer();

public:
    int BroadCast(char* pData, ssize_t iLen);

protected:
    int OnAccept(uv_tcp_t* pUvTcp);
    int OnInit();
    void OnBaseTimer();

private:
    int AddCli(uv_tcp_t* pUvTcp);

private:
    uv_tcp_t mstSvrTcp;
    std::vector<CWsCli*> mvecClis;
    CUvMutex mcvecClisMutex;
};

#define sWsSvr CWsServer::Instance()
#endif