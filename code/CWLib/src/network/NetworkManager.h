#pragma once

#include <network/NetworkFriendsNP.h>

class CNetworkManager {
public:
    void* Messaging;
    void* InputManager;
    void* ConnectionManager;
    void* GamesManager;
    void* PartiesManager;
    void* GameDataManager;
    void* Unknown;
    CNetworkFriendsManager& FriendsManager;
};

extern CNetworkManager gNetworkManager;
