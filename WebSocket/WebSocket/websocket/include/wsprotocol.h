#ifndef __WSPROTOCOL__H_
#define __WSPROTOCOL__H_
#include <stdint.h>
enum WS_STATE{
    WS_STATE_NONE = 0,
    WS_STATE_CONNECT,
    WS_STATE_UNCONNECT,
};

#define  WS_MIN_MSG_EXPECT_LEN 6
#define WS_KEEP_ALIVE_SEC   1000

enum WS_FRAMETYPE{
    WS_FRAME_EMPTY = 0xF0,
    WS_FRAME_ERROR = 0xF1,
    WS_FRAME_TEXT = 0x01,
    WS_FRAME_BINARY = 0x02,
    WS_FRAME_PING = 0x09,
    WS_FRAME_PONG = 0x0A,
    WS_FRAME_OPENING = 0xF3,
    WS_FRAME_CLOSING = 0x08
};

//FILEDS
#define GET	"GET"
#define WEBSOCKETKEY "Sec-WebSocket-Key"
#define WEBSOCKETPROTOCOL	"Sec-WebSocket-Protocol"
#define WEBSOCKETPINGPONG	"PINGPONG"

//protocol
#define BLANK "blank"
#define ECHO "echo"

#pragma pack(1)
struct tagWsMsgCache {
    char* pData;
    size_t iTotal;
    size_t iUse;
};

struct tagWsMsg {
    char* payload;
    size_t payloadLength;
    uint8_t payloadFieldExtraBytes;
    uint8_t frameType;
};
#pragma pack()
#endif