// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2026 BTS

#ifndef PENDANT_DEFINES_H
#define PENDANT_DEFINES_H

#include <cstdint>

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

enum class CommPacketType: uint8_t {
    STATE = 0,
    WIFI_CONFIG = 1,
    PING = 2,
    STEP_SIZE_CONFIG = 3,
    FEED_RATE_CONFIG = 4,
    CMD = 5,
    STEP_SIZE_CHANGED = 6,
    FEED_RATE_CHANGED = 7,
    JOG = 8,
    MAX,
};

enum class CommunicationMode: uint8_t {
    NONE = 0,
    SERIAL_,
    WIFI,
};

enum class CmdType: uint8_t {
    START = 0,
    STOP,
    PAUSE,
    HOME,
    RESET,
    SPINDLE,
    JOG_STOP,
};

#define COMM_PACKET_VERSION 1
#define COMM_PREAMBLE 0xAA55
#define COMM_START 0xAA55
#define COMM_START_BYTE_1 0x55
#define COMM_START_BYTE_2 0xAA
#define COMM_HEAD_START_1_POS 0
#define COMM_HEAD_START_2_POS 1
#define COMM_HEAD_VERSION_POS 2
#define COMM_HEAD_SIZE_POS 3
#define COMM_HEAD_TYPE_POS 4
#define COMM_SSID_PASSWORD_MAX_LEN 32

PACK(struct CommHeader
{
    uint16_t start = COMM_PREAMBLE;
    uint8_t version = COMM_PACKET_VERSION;
    uint8_t size;
    uint8_t type;
});

PACK(struct CommFooter
{
    uint8_t crc;
});

PACK(struct StateMessage
{
    CommHeader header;
    float x;
    float y;
    float z;
    uint8_t machineState;
    CommunicationMode mode;
    char selectedAxis;
    CommFooter footer;
});

PACK(struct WifiConfigMessage
{
    CommHeader header;
    char ssid[COMM_SSID_PASSWORD_MAX_LEN];
    char password[COMM_SSID_PASSWORD_MAX_LEN];
    char clientIp[16];
    CommFooter footer;
});

#define CHOICES_COUNT 12

PACK(struct StepSizeConfigMessage
{
    CommHeader header;
    float selections[CHOICES_COUNT];
    bool separateZ;
    CommFooter footer;
});

PACK(struct PingMessage
{
    CommHeader header;
    float selections[CHOICES_COUNT];
    bool separateZ;
    CommFooter footer;
});

PACK(struct FeedRateConfigMessage
{
    CommHeader header;
    float selections[CHOICES_COUNT];
    bool separateZ;
    CommFooter footer;
});

PACK(struct CmdMessage
{
    CommHeader header;
    CmdType cmd;
    CommFooter footer;
});

PACK(struct StepSizeChangedMessage
{
    CommHeader header;
    float value;
    CommFooter footer;
});

PACK(struct FeedRateChangedMessage
{
    CommHeader header;
    float value;
    CommFooter footer;
});

PACK(struct JogMessage
{
    CommHeader header;
    int8_t x;
    int8_t y;
    int8_t z;
    CommFooter footer;
});

template <typename T>
T vmax(T a) {
    return a;
}

template <typename T, typename... Args>
T vmax(T a, Args... args) {
    T b = vmax(args...);

    return (a > b) ? a : b;
}

#define COMM_MAX_PACKET_SIZE vmax(sizeof(StateMessage), \
        sizeof(WifiConfigMessage), sizeof(PingMessage), \
        sizeof(StepSizeConfigMessage), sizeof(FeedRateConfigMessage), sizeof(CmdMessage), \
        sizeof(StepSizeChangedMessage), sizeof(FeedRateChangedMessage), sizeof(JogMessage) \
)

#endif // PENDANT_DEFINES_H
