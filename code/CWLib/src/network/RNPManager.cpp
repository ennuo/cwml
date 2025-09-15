#include "network/RNPManager.h"

CPlayerErrorData* CRNPManager::FindErrorData(const NetworkPlayerID& player_id)
{
    for (int i = 0; i < MAX_PLAYERS; ++i)
    {
        CPlayerErrorData* err = PlayerErrorData + i;
        if (err->IsValid() && PlayerIDsMatch(err->PlayerID, player_id))
            return err;
    }

    return NULL;
}