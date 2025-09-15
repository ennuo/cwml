#include <CrossPlay.h>

#include <rpcn/Client.h>
#include <rpcn/SignalingHandler.h>
#include <rpcn/NPHandler.h>

#include <network/NetworkUtilsNP.h>
#include <network/NetworkFriendsNP.h>
#include <np.h>
#include <CritSec.h>
#include <DebugLog.h>
#include <thread.h>
#include <wolfssl/ssl.h>
#include <netdb.h>
#include <new>
#include <vector.h>
#include <MMString.h>
#include <map>
#include <filepath.h>
#include <Clock.h>
#include <sys/poll.h>
#include <Endian.h>

namespace crossplay 
{
    using namespace rpcn;

    namespace signaling 
    {
        int GetConnectionInfo(u32 ctx_id, u32 conn_id, s32 code, SceNpSignalingConnectionInfo& info)
        {
            signaling_info* si;
            if (Signaling != NULL && (si = Signaling->GetSignalingInfo(conn_id)) != NULL)
            {
                switch (code)
                {
                    case SCE_NP_SIGNALING_CONN_INFO_RTT:
                    {
                        info.rtt = si->RTT;
                        break;
                    }
                    case SCE_NP_SIGNALING_CONN_INFO_BANDWIDTH:
                    {
                        info.bandwidth = 100000000;
                        break;
                    }
                    case SCE_NP_SIGNALING_CONN_INFO_PEER_NPID:
                    {
                        info.npId = si->NpId;
                        break;
                    }
                    case SCE_NP_SIGNALING_CONN_INFO_PEER_ADDRESS:
                    {
                        info.address.port = si->Port;
                        info.address.addr.s_addr = si->Address;
                        break;
                    }
                    case SCE_NP_SIGNALING_CONN_INFO_MAPPED_ADDRESS:
                    {
                        info.address.port = si->MappedPort;
                        info.address.addr.s_addr = si->MappedAddress;
                        break;
                    }
                    case SCE_NP_SIGNALING_CONN_INFO_PACKET_LOSS:
                    {
                        info.packet_loss = 0;
                        break;
                    }
                    default:
                    {
                        return SCE_NP_SIGNALING_ERROR_INVALID_ARGUMENT;
                    }
                }

                return CELL_OK;
            }

            return sceNpSignalingGetConnectionInfo(ctx_id, conn_id, code, &info);
        }

        int GetConnectionStatus(u32 ctx_id, u32 conn_id, s32& conn_status, in_addr* peer_addr, in_port_t* peer_port)
        {
            signaling_info* si;
            if (Signaling != NULL && (si = Signaling->GetSignalingInfo(conn_id)) != NULL)
            {
                conn_status = si->Status;
                if (peer_addr) peer_addr->s_addr = si->Address;
                if (peer_port) *peer_port = si->Port;

                return CELL_OK;
            }

            return sceNpSignalingGetConnectionStatus(ctx_id, conn_id, &conn_status, peer_addr, peer_port);
        }

        int CreateContext(const NetworkPlayerID& np_id, SceNpSignalingHandler handler, void* arg, u32& ctx_id)
        {
            SignalingCallback = handler;
            SignalingCallbackArgument = arg;
            Signaling->SetNpId(np_id);

            MMLog("crossplay: hijacking np context, handler: %08x, arg: %08x\n", (uintptr_t)handler, (uintptr_t)arg);
            
            return sceNpSignalingCreateCtx(&np_id, handler, arg, &ctx_id);
        }

        int ActivateConnection(u32 ctx_id, const NetworkPlayerID& np_id, u32& conn_id)
        {
            const char* player_name = GetPlayerName(np_id);
            friend_online_data* f;
            if (Client != NULL && (f = Client->FindFriend(player_name)) != NULL)
            {
                MMLog("Activating connection for %s through RPCN\n", player_name);
                conn_id = Signaling->InitSignaling(np_id);
                return CELL_OK;
            }

            MMLog("Activating connection for %s through PSN\n", player_name);
            return sceNpSignalingActivateConnection(ctx_id, &np_id, &conn_id);
        }

        int DeactivateConnection(u32 ctx_id, u32 conn_id)
        {
            if (Signaling != NULL && Signaling->GetSignalingInfo(conn_id) != NULL)
            {
                Signaling->StopSignaling(conn_id, true);
                return CELL_OK;
            }

            return sceNpSignalingDeactivateConnection(ctx_id, conn_id);
        }
    }

    namespace npbasic
    {
        int RegisterContextSensitiveHandler(const SceNpCommunicationId* context, SceNpBasicEventHandler handler, void* arg)
        {
            NPCommunicationID = *context;
            NPBasicEventHandler = handler;
            NPBasicEventArg = arg;

            return sceNpBasicRegisterContextSensitiveHandler(context, handler, arg);
        }

        int GetBasicEvent(int* event, SceNpUserInfo* from, void* data, size_t* size)
        {
            int ret;
            if (NP != NULL && (ret = NP->GetBasicEvent(event, from, data, size)) != SCE_NP_BASIC_ERROR_NO_EVENT)
                return ret;
            return sceNpBasicGetEvent(event, from, data, size);
        }

        int SetPresenceDetails(const SceNpBasicPresenceDetails* pres, u32 options)
        {
            if (Client != NULL)
            {
                ByteArray data;
                MMString<char> status;
                if (options & SCE_NP_BASIC_PRESENCE_OPTIONS_SET_STATUS)
                    status = pres->status;
                
                if (options & SCE_NP_BASIC_PRESENCE_OPTIONS_SET_DATA)
                {
                    data.try_resize(pres->size);
                    memcpy(data.begin(), pres->data, pres->size);
                }

                Client->SetPresence(status, data);
            }

            return sceNpBasicSetPresenceDetails(pres, options);
        }
    }

    int CheckCallbacks()
    {
        CheckSignalingCallbacks();
        CheckNPCallbacks();
        return cellSysutilCheckCallback();
    }

    bool Initialise()
    {
        if (!gHost.IsSet())
        {
            MMLog("crossplay: not starting rpcn client because server address isn't set!\n");
            return false;
        }
        
        if (!gNPID.IsSet())
        {
            MMLog("crossplay: not starting rpcn client because username isn't set!\n");
            return false;
        }

        if (!gPassword.IsSet())
        {
            MMLog("crossplay: not starting rpcn client because password isn't set!\n");
            return false;
        }

        if (!gToken.IsSet())
        {
            MMLog("crossplay: not starting rpcn client because token isn't set!\n");
            return false;
        }
        
        RemoteMessages.try_reserve(32);
        NP = new nphandler();
        Signaling = new sighandler();
        Client = new client();

        return true;
    }

    void Close()
    {
        if (Client != NULL) delete Client;
        if (Signaling != NULL) delete Signaling;
        if (NP != NULL) delete NP;
        
        Signaling = NULL;
        Client = NULL;
        NP = NULL;

        CCSLock lock(&RemoteMessageMutex, __FILE__, __LINE__);
        for (RemoteMessage* it = RemoteMessages.begin(); it != RemoteMessages.end(); ++it)
            delete it->Data;
        RemoteMessages.clear();
    }
}
