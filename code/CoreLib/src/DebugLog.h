#pragma once

const u32 MAX_MMLOGCH_STRING_LENGTH = 0x200;
extern bool gDebugLoggingEnabled;

enum EDebugChannel 
{
    DC_STDOUT,
    DC_DEFAULT,
    DC_SYSTEM,
    DC_RESOURCE,
    DC_PLAYER_PROFILE,
    DC_NETWORK,
    DC_SCRIPT,
    DC_AC,
    DC_PUBLISH,
    DC_HTTP,
    DC_GRAPHICS,
    DC_WEBCAM,
    DC_LOCALISATION,
    DC_INIT,
    DC_BUILD,
    DC_VOIP,
    DC_TESTSUITE,
    DC_REPLAY,

    NUM_DEBUG_CHANNELS
};

struct DebugChannelOptions
{
    u32 TTY;
    bool Enabled;
    const char* INI;
};

void DebugLogChR(EDebugChannel channel, char* b, char* e);
void DebugLogChF(EDebugChannel channel, const char* format, ...);
void DebugLogChV(EDebugChannel channel, const char* format, va_list args);
void DebugLogEnable(EDebugChannel channel, bool enable);

#define MMLogCh DebugLogChF
#define MMLog(...) DebugLogChF(DC_STDOUT, __VA_ARGS__);
