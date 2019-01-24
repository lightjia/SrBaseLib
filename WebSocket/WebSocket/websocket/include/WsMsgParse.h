#ifndef __CWSMSGPARSE__H_
#define __CWSMSGPARSE__H_
#include "wsprotocol.h"
#include "CLogmanager.h"
class CWsMsgParse : public CSingleton<CWsMsgParse>{
    SINGLE_CLASS_INITIAL(CWsMsgParse);
public:
    ~CWsMsgParse();

public:
    int DecodeMsg(tagWsMsgCache* pMsgCache, tagWsMsg** pWsMsg);
    len_str EncodeMsg(const char* pData, const size_t iLen, int iFrameType);
};

#define sWsMsgParse CWsMsgParse::Instance()
#endif