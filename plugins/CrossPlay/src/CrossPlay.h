#pragma once

#include <MMString.h>
#include <vector.h>
#include <network/NetworkUtilsNP.h>
#include <np.h>

#include <rpcn/Types.h>


namespace crossplay
{
    namespace rpcn
    {
        extern int StartupTimeout;
    }

    namespace signaling
    {
        int GetConnectionInfo(u32 ctx_id, u32 conn_id, s32 code, SceNpSignalingConnectionInfo& info);
        int GetConnectionStatus(u32 ctx_id, u32 conn_id, s32& conn_status, in_addr* peer_addr, in_port_t* peer_port);
        int CreateContext(const NetworkPlayerID& np_id, SceNpSignalingHandler handler, void* arg, u32& ctx_id);
        int ActivateConnection(u32 ctx_id, const NetworkPlayerID& np_id, u32& conn_id);
        int DeactivateConnection(u32 ctx_id, u32 conn_id);
    }

    namespace npbasic
    {
        int RegisterContextSensitiveHandler(const SceNpCommunicationId* context, SceNpBasicEventHandler handler, void* arg);
        int GetBasicEvent(int* event, SceNpUserInfo* from, void* data, size_t* size);
    }

    int CheckCallbacks();
    bool Initialise();
    void Close();
}
