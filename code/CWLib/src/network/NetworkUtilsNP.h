#pragma once

#ifdef WIN32

#define	SCE_NET_NP_ONLINEID_MAX_LENGTH		16

typedef struct SceNpOnlineId {
	char data[SCE_NET_NP_ONLINEID_MAX_LENGTH];
	char term;
	char dummy[3];
} SceNpOnlineId;
typedef struct SceNpOnlineId SceNpPsHandle; 

/* NP ID */
typedef struct SceNpId {
	SceNpOnlineId handle;
	uint8_t opt[8];
	uint8_t reserved[8];
} SceNpId;

typedef SceNpId NetworkPlayerID;
typedef SceNpOnlineId NetworkOnlineID;

#else

#include <np/common.h>
#include <arpa/inet.h>

#define MAX_PLAYERS (4)

typedef SceNpAvatarUrl NetworkAvatarURL;
typedef SceNpId NetworkPlayerID;
typedef SceNpOnlineId NetworkOnlineID;

typedef in_addr_t NetworkIPAddress;
typedef in_port_t NetworkPort;

extern NetworkPlayerID INVALID_PLAYER_ID;

enum ENpIdSource {
    NPID_SOURCE_PSN,
    NPID_SOURCE_RPCN,
    NPID_SOURCE_P2P
};

bool PlayerIDsMatch(const SceNpId& a, const SceNpId& b);
const char* GetPlayerName(const SceNpId& id);
const char* GetPlayerName(const SceNpOnlineId& id);
bool operator==(const NetworkPlayerID& a, const NetworkPlayerID& b);

ENpIdSource GetPlayerSource(const NetworkPlayerID& id);
void SetPlayerSource(NetworkPlayerID& id, ENpIdSource source);

#endif // WIN32