#ifndef __CWSMSG__H_
#define __CWSMSG__H_
#include "RcObject.h"
#include "wsprotocol.h"
#include "WsCli.h"
#include <string>

class CWsMsg : public CRcObject{
public:
    CWsMsg(std::string& strProtocol, CWsCli* pCli);
    ~CWsMsg();

public:
    std::string& GetProtocol() { return mstrProtocol; }
    void SetMsg(tagWsMsg* pMsg);
    tagWsMsg* GetMsg() { return mpMsg; }
    CWsCli* GetCli();
    void SetHttpMsg(std::string& strHttpMsg) { mstrHttpMsg = strHttpMsg; }
    std::string& GetHttpMsg() { return mstrHttpMsg; }

private:
    std::string mstrProtocol;
    std::string mstrHttpMsg;
    tagWsMsg* mpMsg;
    CWsCli* mpCli;
};
#endif