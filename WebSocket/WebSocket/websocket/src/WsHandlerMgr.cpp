#include "WsHandlerMgr.h"
#include "WsGetHandler.h"
#include "WsUpgradeHandler.h"
#include "WsEchoHandler.h"
CWsHandlerMgr::CWsHandlerMgr(){
    mbInit = false;
}

CWsHandlerMgr::~CWsHandlerMgr(){
}

int CWsHandlerMgr::Init() {
    if (!mbInit) {
        std::pair<std::map<std::string, CWsHandler*>::iterator, bool> pair_handler;
        pair_handler = mmapHandlers.insert(std::make_pair(GET, new CWsGetHandler()));
        ASSERT_RET_VALUE(pair_handler.second, 1);
        pair_handler = mmapHandlers.insert(std::make_pair(WEBSOCKETKEY, new CWsUpgradeHandler()));
        ASSERT_RET_VALUE(pair_handler.second, 1);
        pair_handler = mmapHandlers.insert(std::make_pair(ECHO, new CWsEchoHandler()));
        ASSERT_RET_VALUE(pair_handler.second, 1);
        mbInit = true;
    }

    return 0;
}

int CWsHandlerMgr::UnInit() {
    mbInit = false;
    return 0;
}

CWsHandler* CWsHandlerMgr::GetHandler(std::string& strProtocol) {
    ASSERT_RET_VALUE(mbInit, NULL);
    CWsHandler* pHandler = NULL;
    std::map<std::string, CWsHandler*>::iterator iter = mmapHandlers.find(strProtocol);
    if (iter != mmapHandlers.end()) {
        pHandler = iter->second;
    }

    return pHandler;
}

int CWsHandlerMgr::ProcMsg(CWsMsg* pMsg) {
    ASSERT_RET_VALUE(pMsg && mbInit, 1);
    LOG_INFO("Enter CWsHandlerMgr::ProcMsg:%s", pMsg->GetProtocol().c_str());
    CWsHandler* pHandler = GetHandler(pMsg->GetProtocol());
    ASSERT_RET_VALUE(pHandler, 1);

    pHandler->ProcMsg(pMsg);
    DODELETE(pMsg);
    return 0;
}