#pragma once

#include <network/NetworkUtilsNP.h>
#include <vector.h>

class CNetworkConnection {
public:
    inline CNetworkConnection()
    {
        memset(this, 0, sizeof(CNetworkConnection));
        ConnectionID = ~0ul;
    }
public:
    NetworkPlayerID PlayerID;
    u32 ConnectionID;
    s32 ConnectionStatus;
    NetworkIPAddress IPAddress;
    NetworkPort Port;
    bool JustAdded;
    bool JustEstablished;
    bool Active;
    bool Dead;
};

class CNetworkSignallingManager {
enum EPrepareStage {
    E_PREPARE_STAGE_INIT,
    E_PREPARE_STAGE_FULLY_CONNECTED
};
public:
    void StartConnection(const NetworkPlayerID& a);
    void Update();
private:
    u32 NPSignallingContextID;
    EPrepareStage PrepareStage;
    CRawVector<CNetworkConnection> Connections;
    u32 MTU;
};
