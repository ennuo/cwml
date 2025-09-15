#pragma once

#include <refcount.h>
#include "network/NetworkUtilsNP.h"

class CPlayerToAskData {
public:
    NetworkPlayerID PlayerID;
    u32 OffsetAskedFor;
    u32 LengthAskedFor;
    u32 LengthReceived;
    float TimeLastDataArrived;
    bool ThisPlayerDoesntHaveResource;
};

class CRNPLoadingTask {
public:
    inline bool IsValid() const { return CSR != NULL; }
public:
    CP<void*> CSR;
    s32 Priority;
    CPlayerToAskData PlayersToAsk[MAX_PLAYERS];
    u32 IndexIntoPlayersToAsk;
private:
    u32 Length;
    u32 LengthReceived;
};
