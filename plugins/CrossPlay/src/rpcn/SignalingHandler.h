#pragma once

#include <network/NetworkUtilsNP.h>
#include <np.h>
#include <map>
#include <CritSec.h>
#include <vector.h>

namespace crossplay
{
    extern SceNpSignalingHandler SignalingCallback;
    extern void* SignalingCallbackArgument;
    void CheckSignalingCallbacks();
    
    struct signaling_info
    {
        signaling_info();
        void UpdateRTT(u64 rtt_timestamp);
        void UpdateAddress(u32 addr, u16 port);
        void UpdateMappedAddress(u32 addr, u16 port);
        void UpdateStatus(s32 new_status, s32 error_code);
        void UpdateExtendedStatus(bool op_activated);

        s32 Status;
        u32 Address;
        u16 Port;
        u32 MappedAddress;
        u16 MappedPort;
        u64 LastSeen;
        SceNpId NpId;
        u32 ConnectionID;
        bool Activated;
        u32 InfoCounter;
        u64 LastRTTs[6];
        size_t RTTCounters;
        u32 RTT;
        u32 PingsSent;
        u32 LostPings;
        u32 PacketLoss;
    };

    enum
    {
        eSignalingCommand_Ping,
        eSignalingCommand_Pong,
        eSignalingCommand_Connect,
        eSignalingCommand_ConnectAck,
        eSignalingCommand_Confirm,
        eSignalingCommand_Finished,
        eSignalingCommand_FinishedAck,
        eSignalingCommand_SignalInfo
    };

    const char* GetNameForSignalingCommand(int cmd);

    class sighandler {
    public:
        static const u32 REPEAT_CONNECT_DELAY = 200;
        static const u32 REPEAT_PING_DELAY = 500;
        static const u32 REPEAT_FINISHED_DELAY = 500;
        static const u32 REPEAT_INFO_DELAY = 200;
        static const u32 SIGNALING_SIGNATURE = 0x5349474e;
        static const u32 SIGNALING_VERSION = 3;
    private:
        struct signaling_packet
        {
            signaling_packet();
            void swap_endianness();

            u32 signature;
            u32 version;
            u64 timestamp_sender;
            u64 timestamp_receiver;
            u32 command;
            u32 sent_addr;
            u16 sent_port;
            SceNpId npid;
        };

        struct queued_packet
        {
            inline queued_packet()
            {
                memset(this, 0, sizeof(queued_packet));
            }
            
            u64 wakeup_time;
            signaling_packet packet;
            signaling_info* sig_info;
        };
        
        typedef std::map<u32, signaling_info*> tSignalingMap;
        typedef std::map<u64, queued_packet> tPacketQueue;
    public:
        sighandler();
        ~sighandler();
    public:
        u32 InitSignaling(const NetworkPlayerID& npid);
        void StartSignaling(u32 conn_id, u32 addr, u32 port);
        void StopSignaling(u32 conn_id, bool forceful);
        signaling_info* GetSignalingInfo(int conn_id);
        signaling_info* GetSignalingInfoByAddress(const sockaddr_in_p2p& addr);
        
        int GetConnectionID(const NetworkPlayerID& npid);
        void StopAllSignaling();
        void SetNpId(const NetworkPlayerID& npid);
    public:
        void HandleIncomingMessage(u32 addr, u32 port, const CFixedVector<char>& data);
        void HandleQueuedMessages();
    private:
        void SendSignalingPacket(signaling_packet& sp, u32 addr, u16 port) const;
        void QueueSignalingPacket(signaling_packet& sp, signaling_info* si, u64 wakeup_time);
        bool ValidateSignalingPacket(const signaling_packet* sp);
        signaling_info* GetSignalingPointer(const signaling_packet* sp);
        void SetupPing(signaling_info* si, signaling_packet& sp);
        void RetirePacket(signaling_info* si, int cmd);
        void ReschedulePacket(signaling_info* si, int cmd, u64 new_timepoint);
    public:
        void RetireAllPackets(signaling_info* si);
    private:
        void StopSignalingInternal(u32 conn_id, bool forceful);
    private:
        tSignalingMap Peers;
        CVector<queued_packet> QueuedPackets;
        u32 NextPeerID;
        signaling_packet Packet;
        CCriticalSec DataMutex;
    };

    extern sighandler* Signaling;
}