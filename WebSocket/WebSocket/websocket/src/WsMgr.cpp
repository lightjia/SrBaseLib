#include "WsMgr.h"
#include "WsServer.h"
#include "WsHandlerMgr.h"
#include "UvTaskPool.h"
CWsMgr::CWsMgr() {
    mbInit = false;
}

CWsMgr::~CWsMgr() {
}

int CWsMgr::Init() {
    if (!mbInit) {
        mbInit = true;
        sWsSvr->SetNetParam("0.0.0.0", 7768);
        sWsSvr->SetUvLoop(uv_default_loop());
        sWsSvr->Init();
        sWsHandlerMgr->Init();
        sUvTaskPool->Init();
    }

    return 0;
}

int CWsMgr::UnInit() {
    mbInit = false;
    return 0;
}