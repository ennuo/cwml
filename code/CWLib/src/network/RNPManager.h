#pragma once

#include "RandomStream.h"
#include "network/RNPSendingTask.h"
#include "network/RNPLoadingTask.h"

class RNPFileDB {};

class CPlayerErrorData {
public:
    inline CPlayerErrorData() : PlayerID(), LastBusyTime(), LastTimeoutTime() {}
    inline CPlayerErrorData(const NetworkPlayerID& player_id) : PlayerID(player_id), LastBusyTime(), LastTimeoutTime() {}
    inline bool IsValid() { return !PlayerIDsMatch(PlayerID, INVALID_PLAYER_ID); }
public:
    NetworkPlayerID PlayerID;
    float LastBusyTime;
    float LastTimeoutTime;
};

class CRNPManager {
enum EPrepareStage {
    E_PREPARE_STAGE_INIT,
    E_PREPARE_STAGE_FULLY_CONNECTED
};
private:
    CPlayerErrorData* FindErrorData(const NetworkPlayerID& player_id);
private:
    EPrepareStage PrepareStage;
    CPlayerErrorData PlayerErrorData[MAX_PLAYERS];
    CRNPLoadingTask LoadingTasks[8];
    CRNPSendingTask SendingTasks[16];
    float TimeToSendNextBlock;
    u32 NextSendTaskIndex;
    CRandomStream RandomNumbers;
    u64 LastUpdateTime;
    RNPFileDB* RNPDB;
};