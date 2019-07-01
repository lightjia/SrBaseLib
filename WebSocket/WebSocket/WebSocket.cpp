// WebSocket.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Log.h"
#include "WsMgr.h"
#include "uv.h"
int main(){
    init_platform();
	tagLogInitParam stLogInitParam;
    sLog->Init(stLogInitParam);
	/*unsigned long iLen = 180000;
	char* pMsg = (char*)do_malloc(iLen + 1);
	for (unsigned long i = 0; i < iLen; ++i) {
		pMsg[i] = 'a' + i % 26;
	}
	for (;;) {
		LOG_INFO("%s", pMsg);
		sleep_ms(2000);
	}*/
    sWsMgr->Init();
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    getchar();
    uninit_platform();
    return 0;
}