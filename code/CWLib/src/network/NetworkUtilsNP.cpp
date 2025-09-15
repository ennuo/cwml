#include "network/NetworkUtilsNP.h"

#include <np/util.h>
#include <np/error.h>

NetworkPlayerID INVALID_PLAYER_ID;

bool PlayerIDsMatch(const SceNpId& id_a, const SceNpId& id_b)
{
    int res = sceNpUtilCmpNpId(&id_a, &id_b);
    if (res == SCE_NP_UTIL_ERROR_INVALID_NP_ID && memcmp(&id_a, &id_b, sizeof(SceNpId)) == 0)
        return true;
    return res == 0;
}

bool PlayerIDsMatch(const SceNpId& id_a, const SceNpOnlineId& id_b)
{
    return false;
}

bool PlayerIDsMatch(const NetworkOnlineID& id_a, const NetworkOnlineID& id_b)
{
    return memcmp(&id_a, &id_b, sizeof(NetworkOnlineID)) == 0;
}

const char* GetPlayerName(const SceNpId& id)
{
    if (PlayerIDsMatch(INVALID_PLAYER_ID, id))
        return "Invalid";
    return GetPlayerName(id.handle);
}

const char* GetPlayerName(const SceNpOnlineId& id)
{
    return id.data;
}

ENpIdSource GetPlayerSource(const NetworkPlayerID& id)
{
    return (ENpIdSource)id.reserved[7];
}

void SetPlayerSource(NetworkPlayerID& id, ENpIdSource source)
{
    id.reserved[7] = source;
}

bool operator==(const NetworkPlayerID& a, const NetworkPlayerID& b)
{
    return PlayerIDsMatch(a, b);
}