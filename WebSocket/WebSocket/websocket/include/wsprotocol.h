#ifndef __WSPROTOCOL__H_
#define __WSPROTOCOL__H_
#include "LocalDefine.h"
#include <stdint.h>

enum WS_STATE{
    WS_STATE_NONE = 0,
    WS_STATE_CONNECT,
    WS_STATE_UNCONNECT,
};

#define  WS_MIN_MSG_EXPECT_LEN 2
#define WS_KEEP_ALIVE_SEC   1000

enum WS_FRAMETYPE{
    WS_FRAME_CONTINUATION = 0x00,
    WS_FRAME_TEXT = 0x01,
    WS_FRAME_BINARY = 0x02,
    WS_FRAME_CLOSING = 0x08,
    WS_FRAME_PING = 0x09,
    WS_FRAME_PONG = 0x0A,
};

//FILEDS
#define GET	"GET"
#define WEBSOCKETKEY "Sec-WebSocket-Key"
#define WEBSOCKETPROTOCOL	"Sec-WebSocket-Protocol"
#define WEBSOCKETPINGPONG	"PINGPONG"

//protocol
#define WS_PROTOCOL_BLANK "blank"
#define WS_PROTOCOL_ECHO "echo"

#pragma pack(1)
struct tagWsMsgFrame{
    uint8_t frameType;
    uint8_t payloadOffset;
    uint8_t mask;
    size_t payloadLength;
    size_t expectsize;
};

struct tagWsMsgCache {
    char* pData;
    size_t iTotal;
    size_t iUse;
    size_t iCurFrameIndex;
    size_t iCurFrameLen;
    size_t iTotalFrameLen;
    int iComplete;
};

struct tagWsMsg {
    char* payload;
    size_t payloadLength;
    uint8_t frameType;
};
#pragma pack()
#endif