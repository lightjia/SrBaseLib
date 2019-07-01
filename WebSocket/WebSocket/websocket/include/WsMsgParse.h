#ifndef __CWSMSGPARSE__H_
#define __CWSMSGPARSE__H_
#include "wsprotocol.h"
#include "Log.h"
class CWsMsgParse : public CSingleton<CWsMsgParse>{
    SINGLE_CLASS_INITIAL(CWsMsgParse);
public:
    ~CWsMsgParse();

public:
    int DecodeMsg(tagWsMsgCache* pMsgCache, tagWsMsg** pWsMsg);
    CMemBuffer* EncodeMsg(CMemBuffer* pMsg, int iFrameType);

private:
    int TryDecodeLen(char* pData, tagWsMsgFrame& stFrame);
};

#define sWsMsgParse CWsMsgParse::Instance()
#endif