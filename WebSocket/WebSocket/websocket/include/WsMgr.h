#ifndef __CWSMGR__H_
#define __CWSMGR__H_
#include "singleton.h"
class CWsMgr : public CSingleton<CWsMgr>{
    SINGLE_CLASS_INITIAL(CWsMgr);
public:
    ~CWsMgr();

public:
    int Init();
    int UnInit();

private:
    bool mbInit;
};

#define sWsMgr CWsMgr::Instance()
#endif