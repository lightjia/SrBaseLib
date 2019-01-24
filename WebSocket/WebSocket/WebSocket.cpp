// WebSocket.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "CLogmanager.h"
#include "WsMgr.h"
#include "uv.h"
int main(){
    sLog->Init(2, 5, ".");
    sWsMgr->Init();
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    getchar();
    return 0;
}