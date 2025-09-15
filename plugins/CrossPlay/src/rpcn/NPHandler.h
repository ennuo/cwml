#pragma once

#include <rpcn/Types.h>

#include <vector.h>
#include <CritSec.h>

namespace crossplay
{
    namespace rpcn { struct friend_online_data; }

    void CheckNPCallbacks();
    
    struct RemoteMessage
    {
        sockaddr_in_p2p Address;
        const char* Data;
        size_t Length;
    };

    struct basic_event
    {
        basic_event()
        {
            memset(this, 0, sizeof(basic_event));
        }
        
        s32 Event;
        SceNpUserInfo From;
        ByteArray Data;
    };

    class nphandler {
    public:
        nphandler();
        ~nphandler();
    public:
        void QueueBasicEvent(basic_event evt);
        bool SendBasicEvent(s32 event, s32 code, u32 request_id);
        int GetBasicEvent(int* event, SceNpUserInfo* from, void* data, size_t* size);
        void SendPresenceUpdate(rpcn::friend_online_data* f);
    private:
        CCriticalSec BasicEventsMutex;
        CVector<basic_event> EventQueue;
    };

    int SendNPPacket(const void* msg, size_t len, int flags, const sockaddr* to, socklen_t to_len);
    bool SendNPPacket(const ByteArray& data, const sockaddr_in& addr);

    ssize_t OnSendP2PPacket(int s, const void* msg, size_t len, int flags, const sockaddr* to, socklen_t to_len);
    ssize_t OnReceiveP2PPacket(int s, void *buf, size_t len, int flags, struct sockaddr* from, socklen_t* fromlen);

    

    extern nphandler* NP;
    extern int NPSocket;
    extern CVector<RemoteMessage> RemoteMessages;
    extern CCriticalSec RemoteMessageMutex;
    extern SceNpCommunicationId NPCommunicationID;
    extern SceNpBasicEventHandler NPBasicEventHandler;
    extern void* NPBasicEventArg;
}