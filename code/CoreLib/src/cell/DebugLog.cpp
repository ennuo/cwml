#include <DebugLog.h>

#include <sys/tty.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

DebugChannelOptions gDebugChannelMapping[] = 
{
    { 0, true, "DC_STDOUT" },
    { 4, true, "DC_DEFAULT" },
    { 4, true, "DC_SYSTEM" },
    { 5, true, "DC_RESOURCE" },
    { 6, true, "DC_PLAYER_PROFILE" },
    { 7, true, "DC_NETWORK" },
    { 8, true, "DC_SCRIPT" },
    { 9, true, "DC_AC" },
    { 10, true, "DC_PUBLISH" },
    { 11, true, "DC_HTTP" },
    { 12, true, "DC_GRAPHICS" },
    { 13, true, "DC_WEBCAM" },
    { 14, true, "DC_LOCALISATION" },
    { 15, true, "DC_INIT" },
    { 4, true, "DC_BUILD" },
    { 4, true, "DC_VOIP" },
    { 3, true, "DC_TESTSUITE"  },
    { 12, true, "DC_REPLAY"  },
};

void DebugLogChR(EDebugChannel channel, char* b, char* e) 
{
    DebugChannelOptions* opt = gDebugChannelMapping + channel;
    if (!opt->Enabled) return;

    unsigned int len;
    sys_tty_write(0, b, e - b, &len);
}

void DebugLogChF(EDebugChannel channel, const char* format, ...) 
{
    va_list args;
    va_start(args, format);
    DebugLogChV(channel, format, args);
    va_end(args);
}

void DebugLogChV(EDebugChannel channel, const char* format, va_list args) 
{
    if (!gDebugChannelMapping[channel].Enabled) return;
    
    char buffer[MAX_MMLOGCH_STRING_LENGTH];
    vsnprintf(buffer, MAX_MMLOGCH_STRING_LENGTH, format, args);
    DebugLogChR(channel, buffer, buffer + strlen(buffer));
}

void DebugLogEnable(EDebugChannel channel, bool enable)
{
    gDebugChannelMapping[channel].Enabled = enable;
}

const char* gNewLine = "\n";
extern "C" void DEBUG_PRINT(const char* format, ...)
{
    // va_list args;
    // va_start(args, format);
    // DebugLogChV(DC_STDOUT, format, args);
    // va_end(args);
    
    // unsigned int len;
    // sys_tty_write(0, gNewLine, 1, &len);
}