#include <rpcn/SignalingHandler.h>
#include <rpcn/Client.h>
#include <rpcn/NPHandler.h>

#include <DebugLog.h>
#include <Clock.h>
#include <Endian.h>
#include <CalendarTime.h>

namespace crossplay
{
    SceNpSignalingHandler SignalingCallback;
    void* SignalingCallbackArgument;
    
    
    struct CallbackData {
        int SubjectID;
        int Event;
        int ErrorCode;
        bool Extended;
    };

    CVector<CallbackData> CallbackQueue;
    CCriticalSec CallbackMutex("CbMtx");

    void AddCallback(int subject_id, int event, int error_code, bool extended)
    {
        CCSLock lock(&CallbackMutex, __FILE__, __LINE__);

        CallbackData cb;
        cb.SubjectID = subject_id;
        cb.Event = event;
        cb.ErrorCode = error_code;

        CallbackQueue.push_back(cb);
    }

    void AddCallback(int subject_id, int event, int error_code) { AddCallback(subject_id, event, error_code, false); }
    void AddExtendedCallback(int subject_id, int event, int error_code) { AddCallback(subject_id, event, error_code, true); }

    void CheckSignalingCallbacks()
    {
        CVector<CallbackData> callbacks;
        {
            CCSLock lock(&CallbackMutex, __FILE__, __LINE__);
            if (CallbackQueue.size() == 0) return;
            callbacks = CallbackQueue;
            CallbackQueue.try_resize(0);
        }

        MMLog("passing %d callbacks to signaling handler\n", callbacks.size());

        for (CallbackData* cb = callbacks.begin(); cb != callbacks.end(); ++cb)
            SignalingCallback(-1, cb->SubjectID, cb->Event, cb->ErrorCode, SignalingCallbackArgument);
    }

    const char* GetNameForSignalingCommand(int cmd)
    {
        switch (cmd)
        {
            case eSignalingCommand_Ping: return "PING";
            case eSignalingCommand_Pong: return "PONG";
            case eSignalingCommand_Connect: return "CONNECT";
            case eSignalingCommand_ConnectAck: return "CONNECT_ACK";
            case eSignalingCommand_Confirm: return "CONFIRM";
            case eSignalingCommand_Finished: return "FINISHED";
            case eSignalingCommand_FinishedAck: return "FINISHED_ACK";
            case eSignalingCommand_SignalInfo: return "INFO";
        }

        return "INVALID";
    }

    signaling_info::signaling_info()
    {
        memset(this, 0, sizeof(signaling_info));
        Status = SCE_NP_SIGNALING_CONN_STATUS_INACTIVE;
        InfoCounter = 10;
        PingsSent = 1;
        LastSeen = GetClockMilliSecondsInt();
    }

    void signaling_info::UpdateRTT(u64 rtt_timestamp)
    {
        u64 timestamp_now = GetCalendarMicroseconds();
        u64 rtt = timestamp_now - rtt_timestamp;
        LastRTTs[RTTCounters % 6] = rtt;
        RTTCounters++;

        size_t num_rtts = MIN(6, RTTCounters);
        u64 sum = 0;
        for (int i = 0; i < num_rtts; ++i)
            sum += LastRTTs[i];

        rtt = sum / num_rtts;
    }

    void signaling_info::UpdateAddress(u32 addr, u16 port)
    {
        Address = addr;
        Port = port;

        MMLog("updating address port to %d\n", Port);
    }

    void signaling_info::UpdateMappedAddress(u32 addr, u16 port)
    {
        MappedAddress = addr;
        MappedPort = port;

        MMLog("updating mapped address port to %d\n", MappedPort);

    }

    void signaling_info::UpdateStatus(s32 new_status, s32 error_code)
    {
        if (Status == SCE_NP_SIGNALING_CONN_STATUS_PENDING && new_status == SCE_NP_SIGNALING_CONN_STATUS_ACTIVE)
        {
            Status = SCE_NP_SIGNALING_CONN_STATUS_ACTIVE;

            AddCallback(ConnectionID, SCE_NP_SIGNALING_EVENT_ESTABLISHED, error_code);
            if (Activated)
                AddExtendedCallback(ConnectionID, SCE_NP_SIGNALING_EVENT_EXT_MUTUAL_ACTIVATED, CELL_OK);
        }
        else if ((Status == SCE_NP_SIGNALING_CONN_STATUS_PENDING || Status == SCE_NP_SIGNALING_CONN_STATUS_ACTIVE) && new_status == SCE_NP_SIGNALING_CONN_STATUS_INACTIVE)
        {
            Status = SCE_NP_SIGNALING_CONN_STATUS_INACTIVE;
            AddCallback(ConnectionID, SCE_NP_SIGNALING_EVENT_DEAD, error_code);
            Signaling->RetireAllPackets(this);
        }
    }

    void signaling_info::UpdateExtendedStatus(bool op_activated)
    {
        if (op_activated && !Activated)
        {
            Activated = true;

            if (Status != SCE_NP_SIGNALING_CONN_STATUS_ACTIVE)
                AddExtendedCallback(ConnectionID, SCE_NP_SIGNALING_EVENT_EXT_PEER_ACTIVATED, CELL_OK);
            else
                AddExtendedCallback(ConnectionID, SCE_NP_SIGNALING_EVENT_EXT_MUTUAL_ACTIVATED, CELL_OK);
        }
        else if (!op_activated && Activated)
        {
            Activated = false;
            AddExtendedCallback(ConnectionID, SCE_NP_SIGNALING_EVENT_EXT_PEER_DEACTIVATED, CELL_OK);
        }
    }

    sighandler::signaling_packet::signaling_packet()
    {
        memset(this, 0, sizeof(signaling_packet));
        signature = SIGNALING_SIGNATURE;
        version = SIGNALING_VERSION;
    }

    void sighandler::signaling_packet::swap_endianness()
    {
        version = swap32(version);
        timestamp_sender = swap64(timestamp_sender);
        timestamp_receiver = swap64(timestamp_receiver);
        command = swap32(command);
        sent_addr = swap32(sent_addr);
        sent_port = swap16(sent_port);
    }
    
    sighandler::sighandler() : Peers(), QueuedPackets(), NextPeerID(0x80000000), Packet(), DataMutex("SigHD") 
    {}

    sighandler::~sighandler()
    {
        StopAllSignaling();
    }

    u32 sighandler::InitSignaling(const NetworkPlayerID& npid)
    {
        CCSLock lock(&DataMutex, __FILE__, __LINE__);

        u32 conn_id = GetConnectionID(npid);
        MMLog("conn_id = %08x\n", conn_id);
        
        signaling_info* peer = GetSignalingInfo(conn_id);
        if (peer == NULL)
        {
            MMLog("Signaling info for peer %s was NULL!\n", GetPlayerName(npid));
            return -1;
        }

        if (peer->Status == SCE_NP_SIGNALING_CONN_STATUS_INACTIVE)
        {
            MMLog("Peer is inactive, requesting signaling info from RPCN\n");
            peer->Status = SCE_NP_SIGNALING_CONN_STATUS_PENDING;

            u64 request_id = rpcn::GetRequestID(rpcn::eRequestId_Misc);
            {
                CCSLock lock(&rpcn::SignalingRequestMutex, __FILE__, __LINE__);
                rpcn::SignalingRequests.insert(rpcn::tSignalInfoRequestsMap::value_type(request_id, conn_id));
            }

            rpcn::Client->RequestSignalingInfos(request_id, GetPlayerName(npid));
        }

        return conn_id;
    }

    void sighandler::StartSignaling(u32 conn_id, u32 addr, u32 port)
    {
        CCSLock lock(&DataMutex, __FILE__, __LINE__);

        signaling_packet& packet = Packet;
        packet.command = eSignalingCommand_Connect;
        packet.timestamp_sender = GetCalendarMicroseconds();

        signaling_info* si = GetSignalingInfo(conn_id);

        u64 now = GetClockMilliSecondsInt();
        si->LastSeen = now;

        if (si->Address == 0 || si->Port == 0)
        {
            si->Address = addr;
            si->Port = port;
        }

        SendSignalingPacket(packet, si->Address, si->Port);
        QueueSignalingPacket(packet, si, now + REPEAT_CONNECT_DELAY);
    }

    signaling_info* sighandler::GetSignalingInfo(int conn_id)
    {
        tSignalingMap::iterator it = Peers.find(conn_id);
        if (it != Peers.end())
            return it->second;
        return NULL;
    }

    signaling_info* sighandler::GetSignalingInfoByAddress(const sockaddr_in_p2p& addr)
    {
        CCSLock lock(&DataMutex, __FILE__, __LINE__);
        tSignalingMap::iterator it = Peers.begin();
        for (; it != Peers.end(); ++it)
        {
            signaling_info* si = it->second;
            if (si->Address == addr.sin_addr.s_addr && si->Port == addr.sin_port)
                return si;
        }

        return NULL;
    }

    int sighandler::GetConnectionID(const NetworkPlayerID& np_id)
    {
        for (tSignalingMap::iterator it = Peers.begin(); it != Peers.end(); ++it)
        {
            if (PlayerIDsMatch(it->second->NpId, np_id))
                return it->first;
        }

        u32 conn = ++NextPeerID;
        signaling_info* info  = new signaling_info();
        info->NpId = np_id;
        info->ConnectionID = conn;
        
        Peers.insert(tSignalingMap::value_type(conn, info));

        return conn;
    }

    void sighandler::SendSignalingPacket(signaling_packet& sp, u32 addr, u16 port) const
    {
        ByteArray packet;

        packet.try_resize(sizeof(signaling_packet) + VPORT_0_HEADER_SIZE);
        *(u16*)&packet[0] = 0;
        packet[2] = eSubset_Signaling;

        sp.sent_addr = addr;
        sp.sent_port = port;

        signaling_packet copy = sp;
        copy.swap_endianness();

        memcpy(packet.begin() + VPORT_0_HEADER_SIZE, &copy, sizeof(signaling_packet));

        sockaddr_in dest;
        memset(&dest, 0, sizeof(sockaddr_in));
        dest.sin_family = AF_INET;
        dest.sin_addr.s_addr = addr;
        dest.sin_port = htons(port);

        char ip_str[16];
        inet_ntop(AF_INET, &dest.sin_addr, ip_str, sizeof(ip_str));

        MMLog("signaling: sending %s packet to %s:%d\n", GetNameForSignalingCommand(sp.command), ip_str, port);

        if (!SendNPPacket(packet, dest))
            MMLog("failed to send signaling packet to %s:%d\n", ip_str, port);
    }

    void sighandler::QueueSignalingPacket(signaling_packet& sp, signaling_info* si, u64 wakeup_time)
    {
        queued_packet qp;
        qp.sig_info = si;
        qp.packet = sp;
        qp.wakeup_time = wakeup_time;

        QueuedPackets.push_back(qp);
    }

    bool sighandler::ValidateSignalingPacket(const signaling_packet* sp)
    {
        if (sp->signature != SIGNALING_SIGNATURE)
        {
            MMLog("signaling: received a signaling packet with an invalid signature\n");
            return false;
        }

        if (sp->version != SIGNALING_VERSION)
        {
            MMLog("signaling: invalid version in signaling packet: %d, expected: %d\n", sp->version, SIGNALING_VERSION);
            return false;
        }

        return true;
    }

    signaling_info* sighandler::GetSignalingPointer(const signaling_packet* sp)
    {
        for (tSignalingMap::iterator it = Peers.begin(); it != Peers.end(); ++it)
        {
            if (PlayerIDsMatch(it->second->NpId, sp->npid))
                return it->second;
        }

        return NULL;
    }

    void sighandler::SetNpId(const NetworkPlayerID& npid)
    {
        CCSLock lock(&DataMutex, __FILE__, __LINE__);
        MMLog("signaling: setting local pid to %s\n", GetPlayerName(npid));
        Packet.npid = npid;
    }

    void sighandler::HandleQueuedMessages()
    {
        CCSLock lock(&DataMutex, __FILE__, __LINE__);

        u64 now = GetClockMilliSecondsInt();
        for (queued_packet* it = QueuedPackets.begin(); it != QueuedPackets.end();)
        {
            u64 timestamp = it->wakeup_time;
            queued_packet& sig = *it;
            signaling_info* si = sig.sig_info;

            if (timestamp > now) break;
            

            int cmd = sig.packet.command;
            if (now - si->LastSeen >= 60000 && cmd != eSignalingCommand_SignalInfo)
            {
                MMLog("signaling: timeout disconnection, last_seen_ms: %llu, now_ms: %llu\n", si->LastSeen, now);
                si->UpdateStatus(SCE_NP_SIGNALING_CONN_STATUS_INACTIVE, SCE_NP_SIGNALING_ERROR_TIMEOUT);
                RetirePacket(si, eSignalingCommand_Ping);
                break;
            }

            switch (sig.packet.command)
            {
                case eSignalingCommand_Connect:
                case eSignalingCommand_Ping:
                    sig.packet.timestamp_sender = GetCalendarMicroseconds();
                    break;
                case eSignalingCommand_ConnectAck:
                    sig.packet.timestamp_receiver = GetCalendarMicroseconds();
                    break;
                default: break;
            }

            SendSignalingPacket(sig.packet, si->Address, si->Port);


            int delay = 500;
            switch (cmd)
            {
                case eSignalingCommand_Ping:
                case eSignalingCommand_Pong:
                    delay = REPEAT_PING_DELAY;
                    break;
                case eSignalingCommand_Connect:
                case eSignalingCommand_ConnectAck:
                case eSignalingCommand_Confirm:
                    delay = REPEAT_CONNECT_DELAY;
                    break;
                case eSignalingCommand_Finished:
                case eSignalingCommand_FinishedAck:
                    delay = REPEAT_FINISHED_DELAY;
                    break;
                case eSignalingCommand_SignalInfo:
                {
                    if (si->InfoCounter == 0)
                    {
                        it = QueuedPackets.erase(it);
                        continue;
                    }

                    delay = REPEAT_INFO_DELAY;
                    si->InfoCounter--;
                    break;
                }
            }

            it++;

            ReschedulePacket(si, cmd, now + delay);
        }




    }

    void sighandler::RetireAllPackets(signaling_info* si)
    {
        if (si == NULL) return;

        for (queued_packet* it = QueuedPackets.begin(); it != QueuedPackets.end();)
        {
            if (it->sig_info == si)
                it = QueuedPackets.erase(it);
            else
                it++;
        }
    }

    void sighandler::RetirePacket(signaling_info* si, int cmd)
    {
        if (si == NULL) return;

        for (queued_packet* it = QueuedPackets.begin(); it != QueuedPackets.end(); ++it)
        {
            if (it->packet.command == cmd && it->sig_info == si)
            {
                QueuedPackets.erase(it);
                return;
            }
        }
    }

    void sighandler::SetupPing(signaling_info* si, signaling_packet& sp)
    {
        if (si == NULL) return;

        for (queued_packet* it = QueuedPackets.begin(); it != QueuedPackets.end(); ++it)
        {
            if (it->packet.command == eSignalingCommand_Ping && it->sig_info == si)
                return;
        }

        sp.command = eSignalingCommand_Ping;
        sp.timestamp_sender = GetCalendarMicroseconds();
        SendSignalingPacket(sp, si->Address, si->Port);
        QueueSignalingPacket(sp, si, GetClockMilliSecondsInt() + REPEAT_PING_DELAY);
    }

    void sighandler::ReschedulePacket(signaling_info* si, int cmd, u64 new_timepoint)
    {
        for (queued_packet* it = QueuedPackets.begin(); it != QueuedPackets.end(); ++it)
        {
            if (it->packet.command == cmd && it->sig_info == si)
            {
                queued_packet qp;
                qp.packet = it->packet;
                qp.sig_info = it->sig_info;
                qp.wakeup_time = new_timepoint;

                QueuedPackets.erase(it);
                QueuedPackets.push_back(qp);
                
                return;
            }
        }
    }

    void sighandler::StopSignaling(u32 conn_id, bool forceful)
    {
        CCSLock lock(&DataMutex, __FILE__, __LINE__);
        StopSignalingInternal(conn_id, forceful);
    }

    void sighandler::StopAllSignaling()
    {
        CCSLock lock(&DataMutex, __FILE__, __LINE__);

        for (tSignalingMap::iterator it = Peers.begin(); it != Peers.end(); ++it)
        {
            StopSignalingInternal(it->first, true);
            delete it->second;
        }

        Peers.clear();
    }

    void sighandler::StopSignalingInternal(u32 conn_id, bool forceful)
    {
        signaling_info* si = GetSignalingInfo(conn_id);
        if (!si) return;

        RetireAllPackets(si);

        if (forceful)
        {
            si->Status = SCE_NP_SIGNALING_CONN_STATUS_INACTIVE;
            si->Activated = false;
        }

        if (si->Status != SCE_NP_SIGNALING_CONN_STATUS_INACTIVE) return;

        signaling_packet& packet = Packet;
        packet.command = eSignalingCommand_Finished;

        SendSignalingPacket(packet, si->Address, si->Port);
        QueueSignalingPacket(packet, si, GetClockMilliSecondsInt() + REPEAT_FINISHED_DELAY);
    }

    void sighandler::HandleIncomingMessage(u32 addr, u32 port, const CFixedVector<char>& data)
    {
        CCSLock lock(&DataMutex, __FILE__, __LINE__);

        if (data.size() != sizeof(signaling_packet))
        {
            MMLog("signaling: received an invalid signaling packet\n");
            return;
        }

        signaling_packet* sp = (signaling_packet*)data.begin();
        sp->swap_endianness();

        if (!ValidateSignalingPacket(sp)) return;

        {
            in_addr inaddr;
            memset(&inaddr, 0, sizeof(in_addr));
            inaddr.s_addr = addr;
            char ip_str[16];
            inet_ntop(AF_INET, &inaddr, ip_str, sizeof(ip_str));
            const char* handle = sp->npid.handle.data;

            MMLog("SP %s from %s:%d (npid: %s, μs: %llu) (our_μs: %llu)\n", GetNameForSignalingCommand(sp->command), ip_str, port, handle, sp->timestamp_sender, GetCalendarMicroseconds());
        }

        bool reply = false, schedule_repeat = false;

        signaling_packet& packet = Packet;
        signaling_info* si = GetSignalingPointer(sp);
        
        if (!si && (sp->command == eSignalingCommand_Connect || sp->command == eSignalingCommand_SignalInfo))
        {
            const u32 conn_id = GetConnectionID(sp->npid);
            si = GetSignalingInfo(conn_id);
        }

        if (!si && sp->command != eSignalingCommand_Finished) return;

        u64 now = GetClockMilliSecondsInt();
        if (si) si->LastSeen = now;

        switch (sp->command)
        {
            case eSignalingCommand_Ping:
            {
                reply = true;
                schedule_repeat = false;
                packet.command = eSignalingCommand_Pong;
                packet.timestamp_sender = sp->timestamp_sender;
                break;
            }
            case eSignalingCommand_Pong:
            {
                if (si)
                    si->UpdateRTT(sp->timestamp_sender);
                reply = false;
                schedule_repeat = false;
                ReschedulePacket(si, eSignalingCommand_Ping, now + 10000);
                break;
            }
            case eSignalingCommand_SignalInfo:
            {
                if (si)
                    si->UpdateAddress(addr, port);
                reply = false;
                schedule_repeat = false;
                break;
            }
            case eSignalingCommand_Connect:
            {
                reply = true;
                schedule_repeat = true;

                packet.command = eSignalingCommand_ConnectAck;
                packet.timestamp_sender = sp->timestamp_sender;
                packet.timestamp_receiver = GetCalendarMicroseconds();

                if (si)
                    si->UpdateAddress(addr, port);

                break;
            }
            case eSignalingCommand_ConnectAck:
            {
                if (si)
                    si->UpdateRTT(sp->timestamp_sender);
                reply = true;
                schedule_repeat = false;
                SetupPing(si, *sp);

                packet.command = eSignalingCommand_Confirm;
                packet.timestamp_receiver = sp->timestamp_receiver;

                RetirePacket(si, eSignalingCommand_Connect);

                if (si)
                {
                    si->UpdateAddress(addr, port);
                    si->UpdateMappedAddress(sp->sent_addr, sp->sent_port);
                    si->UpdateStatus(SCE_NP_SIGNALING_CONN_STATUS_ACTIVE, CELL_OK);
                }

                break;
            }
            case eSignalingCommand_Confirm:
            {
                if (si)
                    si->UpdateRTT(sp->timestamp_receiver);
                reply = false;
                schedule_repeat = false;

                SetupPing(si, *sp);
                RetirePacket(si, eSignalingCommand_ConnectAck);

                if (si)
                {
                    si->UpdateAddress(addr, port);
                    si->UpdateMappedAddress(sp->sent_addr, sp->sent_port);
                    si->UpdateExtendedStatus(true);
                }

                break;
            }
            case eSignalingCommand_Finished:
            {
                reply = true;
                schedule_repeat = false;
                packet.command = eSignalingCommand_FinishedAck;
                if (si)
                {
                    si->UpdateExtendedStatus(false);
                    si->UpdateStatus(SCE_NP_SIGNALING_CONN_STATUS_INACTIVE, SCE_NP_SIGNALING_ERROR_TERMINATED_BY_PEER);
                }
                break;
            }
            case eSignalingCommand_FinishedAck:
            {
                reply = false;
                schedule_repeat = false;
                if (si)
                    si->UpdateStatus(SCE_NP_SIGNALING_CONN_STATUS_INACTIVE, SCE_NP_SIGNALING_ERROR_TERMINATED_BY_MYSELF);
                RetirePacket(si, eSignalingCommand_Finished);
                break;
            }
            default:
            {
                MMLog("signaling: invalid signaling command received\n");
                return;
            }
        }

        if (reply)
        {
            SendSignalingPacket(packet, addr, port);
            if (si && schedule_repeat)
                QueueSignalingPacket(packet, si, now + REPEAT_CONNECT_DELAY);
        }
    }

    sighandler* Signaling;
};