#include <rpcn/NPHandler.h>
#include <rpcn/Client.h>
#include <rpcn/SignalingHandler.h>

#include <DebugLog.h>

namespace crossplay
{
    using namespace rpcn;

    nphandler* NP;
    int NPSocket;

    CCriticalSec RemoteMessageMutex("ReMsg");
    CVector<RemoteMessage> RemoteMessages;

    SceNpCommunicationId NPCommunicationID;
    SceNpBasicEventHandler NPBasicEventHandler;
    void* NPBasicEventArg;

    struct basic_event_cb {
        s32 Event;
        s32 ReturnCode;
        u32 RequestID;
    };

    CVector<basic_event_cb> NPCallbackQueue;
    CCriticalSec NPCallbackMutex("NpCbMtx");

    void CheckNPCallbacks()
    {
        // If the event handler hasn't been added yet, just ignore it for now.
        if (NPBasicEventHandler == NULL) return;

        CVector<basic_event_cb> callbacks;
        {
            CCSLock lock(&NPCallbackMutex, __FILE__, __LINE__);
            if (NPCallbackQueue.size() == 0) return;
            callbacks = NPCallbackQueue;
            NPCallbackQueue.try_resize(0);
        }

        MMLog("passing %d callbacks to np handler\n", callbacks.size());

        for (basic_event_cb* cb = callbacks.begin(); cb != callbacks.end(); ++cb)
            NPBasicEventHandler(cb->Event, cb->ReturnCode, cb->RequestID, NPBasicEventArg);
    }

    nphandler::nphandler() : BasicEventsMutex("EvtMtx"), EventQueue()
    {
        EventQueue.try_reserve(32);

        sockaddr_in in;
        memset(&in, 0, sizeof(sockaddr_in));
        in.sin_family = AF_INET;
        in.sin_port = htons(SCE_NP_PORT);

        NPSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (NPSocket == -1)
        {
            MMLog("np: failed to create udp socket!\n");
            return;
        }

        if (bind(NPSocket, (sockaddr*)&in, sizeof(sockaddr_in)) == 0)
        {
            MMLog("np: created udp socket\n");
            int nb = 1;
            if (setsockopt(NPSocket, SOL_SOCKET, SO_NBIO, &nb, sizeof(int)) != 0)
            {
                MMLog("np: failed to set udp socket to nbio\n");
            }

            sockaddr_in client_addr;
            socklen_t client_addr_size = sizeof(client_addr);
            if (getsockname(NPSocket, (sockaddr*)&client_addr, &client_addr_size) != 0)
            {
                MMLog("np: Failed to get the client address from the socket!\n");
            }

            int localaddr = client_addr.sin_addr.s_addr;
            MMLog("np: np local addr %d.%d.%d.%d:%d\n", 
                (localaddr >> 0) & 0xff,
                (localaddr >> 8) & 0xff,
                (localaddr >> 16) & 0xff,
                (localaddr >> 24) & 0xff,
                client_addr.sin_port
            );
        }
        else
        {

        }
    }

    nphandler::~nphandler()
    {
        if (NPSocket)
        {
            shutdown(NPSocket, SHUT_RDWR);
            socketclose(NPSocket);
            NPSocket = 0;
        }
    }

    void nphandler::QueueBasicEvent(basic_event evt)
    {
        CCSLock lock(&BasicEventsMutex, __FILE__, __LINE__);
        EventQueue.push_back(evt);
    }

    bool nphandler::SendBasicEvent(s32 event, s32 code, u32 request_id)
    {
        if (NPBasicEventHandler == NULL) return false;

        basic_event_cb cb;
        cb.Event = event;
        cb.ReturnCode = code;
        cb.RequestID = request_id;

        CCSLock lock(&NPCallbackMutex, __FILE__, __LINE__);
        NPCallbackQueue.push_back(cb);
    }

    int nphandler::GetBasicEvent(int* event, SceNpUserInfo* from, void* data, size_t* size)
    {
        basic_event evt;
        {
            CCSLock lock(&BasicEventsMutex, __FILE__, __LINE__);
            if (EventQueue.empty()) return SCE_NP_BASIC_ERROR_NO_EVENT;
            evt = EventQueue.front();
            EventQueue.erase(EventQueue.begin());
        }

        *event = evt.Event;
        memcpy(from, &evt.From, sizeof(SceNpUserInfo));
        if (evt.Event != SCE_NP_BASIC_EVENT_OFFLINE)
        {
            const u32 clamped_size = MIN(evt.Data.size(), *size);
            memcpy(data, evt.Data.begin(), clamped_size);
            *size = clamped_size;

            if (clamped_size < evt.Data.size()) 
                return SCE_NP_BASIC_ERROR_DATA_LOST;
        }

        return CELL_OK;
    }

    void nphandler::SendPresenceUpdate(rpcn::friend_online_data* f)
    {
        if (NPBasicEventHandler == NULL) return;

        basic_event evt;
        memset(&evt, 0, sizeof(basic_event));
        strcpy(evt.From.userId.handle.data, f->Name.c_str());
        strcpy(evt.From.name.data, f->Name.c_str());
        
        if (memcmp(f->CommunicationId.data, NPCommunicationID.data, sizeof(f->CommunicationId.data)) == 0)
        {
            evt.Event = SCE_NP_BASIC_EVENT_PRESENCE;
            evt.Data = f->PresenceData;
        }
        else
        {
            evt.Event = SCE_NP_BASIC_EVENT_OUT_OF_CONTEXT;
        }

        QueueBasicEvent(evt);
        SendBasicEvent(evt.Event, 0, 0);
    }

    int SendNPPacket(const void* msg, size_t len, int flags, const sockaddr* to, socklen_t to_len)
    {
        return sendto(NPSocket, msg, len, flags, to, to_len);
    }

    bool SendNPPacket(const ByteArray& data, const sockaddr_in& addr)
    {
        int ret = sendto(NPSocket, data.begin(), data.size(), 0, (sockaddr*)&addr, sizeof(sockaddr_in));
        return ret != -1;
    }

	ssize_t OnReceiveP2PPacket(int s, void *buf, size_t len, int flags, struct sockaddr* from, socklen_t* fromlen)
    {
        int res = recvfrom(s, buf, len, flags, from, fromlen);
        if (res <= 0)
        {
            CCSLock lock(&RemoteMessageMutex, __FILE__, __LINE__);
            if (RemoteMessages.size() != 0)
            {
                RemoteMessage msg = RemoteMessages.front();
                RemoteMessages.erase(RemoteMessages.begin());

                // just assuming the packets arent huge
                memcpy(buf, msg.Data, msg.Length);
                memcpy(from, &msg.Address, sizeof(sockaddr_in_p2p));
                *fromlen = sizeof(sockaddr_in_p2p);

                delete msg.Data;
                return msg.Length;
            }

            return SYS_NET_ERROR_EWOULDBLOCK;
        }

        return res;
    }




    ssize_t OnSendP2PPacket(int s, const void* msg, size_t len, int flags, const sockaddr* to, socklen_t to_len)
    {
        if (to_len != sizeof(sockaddr_in_p2p))
            return sendto(s, msg, len, flags, to, to_len);

        sockaddr_in_p2p* p2p = (sockaddr_in_p2p*)to;

        if (
            // If we're sending packets to ourselves, we obviously don't have to convert to RPCS3
            p2p->sin_addr.s_addr != INADDR_LOOPBACK && 
            p2p->sin_addr.s_addr != INADDR_ANY &&

            // Check if we have this player in our signaling information, if we do,
            // it probably means they're an RPCS3 player and we need to reformat our packets.
            Client != NULL && Signaling != NULL && 
            Signaling->GetSignalingInfoByAddress(*p2p) != NULL
        )
        {

            const int RPCS3_P2P_HEADER_SIZE = sizeof(u16) + sizeof(u16) + sizeof(u16);

            char buf[9216];
            *((u16*)buf) = swap16(p2p->sin_vport);
            *((u16*)(buf + 2)) = swap16(SCE_NP_PORT);
            *((u16*)(buf + 4)) = swap16(0x01); // p2p udp flag
            memcpy(buf + 6, msg, len);

            len += RPCS3_P2P_HEADER_SIZE;
            return SendNPPacket(buf, len, flags, to, to_len);
        }

        return sendto(s, msg, len, flags, to, to_len);
    }
}