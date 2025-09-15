#include <network/NetworkSignalling.h>
#include <np.h>

void CNetworkSignallingManager::StartConnection(const NetworkPlayerID& player)
{
    // Check if the player is already in our list of connections
    for (CNetworkConnection* conn = Connections.begin(); conn != Connections.end(); ++conn)
    {
        if (PlayerIDsMatch(player, conn->PlayerID))
            return;
    }

    // Otherwise we'll create a new connection, snuck a boolean for whether a player ID is supposed to be
    // from RPCN in the reserved field, so we can use that for additional handling.

    CNetworkConnection conn;
    conn.PlayerID = player;
    conn.JustAdded = true;
    Connections.push_back(conn);

}

void CNetworkSignallingManager::Update()
{
    for (CNetworkConnection* conn = Connections.begin(); conn != Connections.end(); ++conn)
    {
        if (conn->JustAdded)
        {
            int ret = sceNpSignalingActivateConnection(NPSignallingContextID, &conn->PlayerID, &conn->ConnectionID);
            //if (ret != CELL_OK && gNetworkManager->GetLastError() != 0)
            //    gNetworkManager->SetLastError(ret);

            conn->JustAdded = false;
        }



    }
}