#pragma once

#include <GuidHash.h>
#include "network/NetworkUtilsNP.h"
#include "Fart.h"

class CRNPSendingTask {
private:
    CHash Hash;
    SResourceReader Handle;
    const char* Relative;
    NetworkPlayerID Requester;
    u32 OriginalBlockOffset;
    u32 LengthRequested;
    u32 OffsetSent;
    char DataToSend[1300];
};
