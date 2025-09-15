#include <rpcn/Client.h>
#include <rpcn/SignalingHandler.h>
#include <rpcn/NPHandler.h>

#include <CrossPlay.h>

#include <network/NetworkUtilsNP.h>
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
    class vec_stream {
    public:
        inline vec_stream(ByteArray& v) : vec(v), i() {}
    public:
        MMString<char> get_string()
        {
            int len = 0;
            const char* ch = vec.begin() + i;

            while (i < vec.size() && vec[i] != '\0')
            {
                i++;
                len++;
            }

            i++;

            return MMString<char>(ch, ch + len);
        }

        template <typename T>
        T get()
        {
            T res = *(T*)&vec[i];
            
            if (sizeof(T) == 2) *((u16*)&res) = swap16(*((u16*)&res));
            else if (sizeof(T) == 4) *((u32*)&res) = swap32(*((u32*)&res));
            else if (sizeof(T) == 8) *((u64*)&res) = swap64(*((u64*)&res));

            i += sizeof(T);

            return res;
        }

        ByteArray get_rawdata()
        {
            u32 size = get<u32>();
            ByteArray b;
            b.try_resize(size);
            memcpy(b.begin(), vec.begin() + i, size);
            i += size;
            return b;
        }

        SceNpCommunicationId get_com_id()
        {
            SceNpCommunicationId com;
            memcpy(&com.data[0], &vec[i], COMMUNICATION_ID_COMID_COMPONENT_SIZE);

            // dont have a strtoul so we're just gonna ignore it i guess
            i += COMMUNICATION_ID_SIZE;

            return com;
        }

    private:
        ByteArray& vec;
        size_t i;
    };

    namespace rpcn
    {
        int MatchReqID = 1;
        int StartupTimeout = 0;

        cwml::ConfigOption<const char*> gHost("RPCN Server", "host", NULL);
        cwml::ConfigOption<const char*> gNPID("RPCN Account Username", "username", NULL);
        cwml::ConfigOption<const char*> gPassword("RPCN Account Password", "password", NULL);
        cwml::ConfigOption<const char*> gToken("RPCN Account Token", "token", NULL);

        tSignalInfoRequestsMap SignalingRequests;
        CCriticalSec SignalingRequestMutex("SigReq");

        u32 GetRequestID(u16 high)
        {
            return high << 16 | MatchReqID++;
        }

        client::client() 
        {
            memset(this, 0, sizeof(client));
            ReadThread = CreatePPUThread(&client::ReaderThreadStatic, (u64)this, "RPCN Reader", 1000, 0x10000, true);
            WriteThread = CreatePPUThread(&client::WriterThreadStatic, (u64)this, "RPCN Writer", 1000, 0x10000, true);
            Thread = CreatePPUThread(&client::MainThreadStatic, (u64)this, "RPCN Client", 1000, 0x10000, true);
            new (&ReadMutex) CCriticalSec("Read");
            new (&WriteMutex) CCriticalSec("Write");
            new (&PacketSendMutex) CCriticalSec("Send");
            new (&RepliesMutex) CCriticalSec("Reply");
            new (&Replies) tReplyMap();
            RequestCounter = 0x100000001ull;

            WantConnection = true;
            WantAuthentication = true;

            new (&MyPresence.Status) MMString<char>();
            new (&MyPresence.Title) MMString<char>();
        }

        client::~client() 
        { 
            TerminateConnection();
            Terminate = true;

            JoinPPUThread(Thread, NULL);
            JoinPPUThread(ReadThread, NULL);
            JoinPPUThread(WriteThread, NULL);

            Disconnect();
        }

        void client::ReaderThreadStatic(u64 args) { ((client*)args)->ReaderThread(); ExitPPUThread(0); }
        void client::WriterThreadStatic(u64 args) { ((client*)args)->WriterThread(); ExitPPUThread(0); }
        void client::MainThreadStatic(u64 args) { ((client*)args)->MainThread(); ExitPPUThread(0); }

        void client::ReaderThread()
        {
            while (true)
            {
                if (Terminate)
                {
                    MMLog("rpcn: exiting reader thread due to termination\n");
                    return;
                }

                if (Connected && !HandleInput()) break;

                ThreadSleep(10);
            }
            
            MMLog("rpcn: exiting reader thread due to error\n");
        }

        void client::WriterThread()
        {
            while (true)
            {
                if (Terminate)
                {
                    MMLog("rpcn: exiting writer thread due to termination\n");
                    return;
                }

                if (Connected && !HandleOutput()) break;

                ThreadSleep(10);
            }

            MMLog("rpcn: exiting writer thread due to error\n");
        }

        void client::MainThread()
        {
            u64 last_ping_time = 0;
            u64 last_pong_time = 0;
            u64 first_pong_time = 0;

            bool first_ping = true;
            bool do_np_test = true;

            char p2p_recv_data[9216];

            ThreadSleep(StartupTimeout);


            while (true)
            {
                if (Terminate)
                {
                    MMLog("rpcn: exiting main thread\n");
                    return;
                }

                if (!Connected && WantConnection)
                {
                    WantConnection = false;
                    Connect(gHost);
                }

                if (Connected && WantAuthentication)
                {
                    WantAuthentication = false;
                    Login(rpcn::gNPID, rpcn::gPassword, rpcn::gToken);
                }
                
                if (Authentified)
                {
                    u64 now = GetClockMilliSecondsInt();

                    if (!first_ping && do_np_test)
                    {
                        // MMLog("???\n");
                        // NetworkPlayerID id;
                        // memset(&id, 0, sizeof(NetworkPlayerID));
                        // strcpy(id.handle.data, "log");
                        // u32 conn_id;
                        // signaling::ActivateConnection(-1, id, conn_id);

                        do_np_test = false;
                    }


                    sockaddr_in native_addr;
                    memset(&native_addr, 0, sizeof(native_addr));
                    socklen_t native_addrlen = sizeof(native_addr);
                    int recv_res = recvfrom(NPSocket, p2p_recv_data, 9216, 0, (sockaddr*)&native_addr, &native_addrlen);

                    if (recv_res >= 2)
                    {
                        u16 dst_vport = swap16(*(u16*)(p2p_recv_data));
                        // MMLog("recv from udp port (vport=%d)\n", dst_vport);
                        if (dst_vport == 0 && recv_res >= VPORT_0_HEADER_SIZE)
                        {
                            u8 subset = p2p_recv_data[2];
                            
                            CFixedVector<char> msg(p2p_recv_data + VPORT_0_HEADER_SIZE, recv_res - VPORT_0_HEADER_SIZE);
                            switch (subset)
                            {
                                case eSubset_RPCN:
                                {
                                    if (msg.size() == 6)
                                    {
                                        PublicAddress = *(u32*)&msg[0];
                                        PublicPort = *(u16*)&msg[4];

                                        // MMLog(
                                        //     "public ip: %d.%d.%d.%d:%d\n",
                                        //     (PublicAddress >> 24) & 0xff,
                                        //     (PublicAddress >> 16) & 0xff,
                                        //     (PublicAddress >> 8) & 0xff,
                                        //     (PublicAddress >> 0) & 0xff,
                                        //     PublicPort
                                        // );

                                        last_pong_time = now;

                                        if (first_ping)
                                        {
                                            first_pong_time = now;
                                            first_ping = false;
                                        }
                                    }

                                    break;
                                }
                                case eSubset_Signaling:
                                {
                                    Signaling->HandleIncomingMessage(
                                        native_addr.sin_addr.s_addr,
                                        native_addr.sin_port,
                                        msg
                                    );
                                    
                                    break;
                                }
                            }

                        }
                        else if (dst_vport == SCE_NP_PORT && recv_res >= 6)
                        {
                            int len = recv_res - 6;
                            char* data = new char[len];
                            memcpy(data, p2p_recv_data + 6, len);

                            // MMLog("Received P2P message w/ len = %d\n", data->size());

                            RemoteMessage remote_msg;
                            memset(&remote_msg.Address, 0, sizeof(sockaddr_in_p2p));
                            remote_msg.Address.sin_len = sizeof(sockaddr_in_p2p);
                            remote_msg.Address.sin_family = AF_INET;
                            remote_msg.Address.sin_addr = native_addr.sin_addr;
                            remote_msg.Address.sin_port = native_addr.sin_port;
                            remote_msg.Address.sin_vport = swap16(*(u16*)(p2p_recv_data + 2));

                            remote_msg.Data = data;
                            remote_msg.Length = len;

                            {
                                CCSLock lock(&RemoteMessageMutex, __FILE__, __LINE__);
                                RemoteMessages.push_back(remote_msg);
                            }
                        }
                    } 
                    // else if (recv_res != SYS_NET_ERROR_EWOULDBLOCK)
                    //     MMLog("nprecv errno: %08x\n", recv_res);

                    u64 time_since_last_ping = now - last_ping_time;
                    u64 time_since_last_pong = now - last_pong_time;

                    if (time_since_last_pong >= 5000 && time_since_last_ping > 500)
                    {

                        ByteArray ping;
                        ping.try_resize(13);
                        ping[0] = 1;
                        *((s64*)&ping[1]) = swap64(UserID);
                        *((u32*)&ping[9]) = LocalAddress;

                        if (!SendNPPacket(ping, NPServerAddr))
                            MMLog("rpcn: failed to send ping packet!\n");
                        // else
                        //     MMLog("rpcn: sent ping packet!\n");
                        
                        last_ping_time = now;
                    }

                    u64 duration =
                        ((now - last_pong_time) < 5000) ?
                        (5000 - (now - last_pong_time)) :
                        (500 - (now - last_ping_time));

                    Signaling->HandleQueuedMessages();
                    // ThreadSleep(duration);
                }

                ThreadSleep(10);
            }

            MMLog("rpcn: exiting main thread due to error\n");
        }

        int client::ReadExactly(u8* buf, size_t n)
        {
            u32 nutimeouts = 0;
            size_t n_recv = 0;
            while (n_recv != n)
            {
                if (!Connected) return eRecv_NoConnection;
                if (Terminate) return eRecv_Terminate;

                int res = wolfSSL_read(WSSLr, ((char*)buf) + n_recv, n - n_recv);
                if (res <= 0)
                {
                    if (wolfSSL_want_read(WSSLr))
                    {
                        pollfd poll_fd;
                        memset(&poll_fd, 0, sizeof(pollfd));

                        while ((poll_fd.revents & POLLIN) != POLLIN && (poll_fd.revents & POLLRDNORM) != POLLRDNORM)
                        {
                            if (!Connected) return eRecv_NoConnection;
                            if (Terminate) return eRecv_Terminate;

                            poll_fd.fd = Socket;
                            poll_fd.events = POLLIN;
                            poll_fd.revents = 0;

                            int res_poll = socketpoll(&poll_fd, 1, TIMEOUT_INTERVAL);
                            if (res_poll < 0)
                            {
                                MMLog("rpcn: recvn poll failed\n");
                                return eRecv_Fatal;
                            }

                            nutimeouts++;

                            if (nutimeouts > (TIMEOUT / TIMEOUT_INTERVAL))
                            {
                                if (n_recv == 0) return eRecv_NoData;
                                MMLog("rpcn: recvn timeout with %d bytes received\n", n_recv);
                                return eRecv_Timeout;
                            }
                        }
                    }
                    else
                    {
                        if (res == 0)
                        {
                            MMLog("rpcn: recvn failed: connection reset by server\n");
                            return eRecv_NoConnection;
                        }

                        MMLog("rpcn: recvn failed with fatal error\n");
                        return eRecv_Fatal;
                    }
                }
                else
                {
                    nutimeouts = 0;
                    n_recv += res;
                }
            }

            return eRecv_Success;
        }
        
        bool client::SendPacket(const ByteArray& packet)
        {
            u32 nutimeouts = 0;
            size_t n_sent = 0;
            while (n_sent != packet.size())
            {
                if (Terminate)
                    return ErrorAndDisconnect("send_packet was forcefully aborted");
                if (!Connected)
                    return false;

                int res = wolfSSL_write(WSSLw, packet.begin() + n_sent, packet.size() - n_sent);
                if (res <= 0)
                {
                    if (wolfSSL_want_write(WSSLw))
                    {
                        pollfd poll_fd;
                        memset(&poll_fd, 0, sizeof(pollfd));

                        while ((poll_fd.revents & POLLOUT) != POLLOUT)
                        {
                            if (Terminate) return ErrorAndDisconnect("send_packet was forcefully aborted");

                            poll_fd.fd = Socket;
                            poll_fd.events = POLLOUT;
                            poll_fd.revents = 0;

                            int res_poll = socketpoll(&poll_fd, 1, TIMEOUT_INTERVAL);
                            if (res_poll < 0)
                            {
                                MMLog("rpcn: send packet failed with native error\n");
                                return ErrorAndDisconnect("send_packet failed on poll");
                            }

                            nutimeouts++;
                            if (nutimeouts > (TIMEOUT / TIMEOUT_INTERVAL))
                            {
                                MMLog("rpcn: send_packet timeout with %d bytes sent\n", n_sent);
                                return ErrorAndDisconnect("failed to send all the bytes");
                            }
                        }
                    }
                    else
                    {
                        return ErrorAndDisconnect("Faield to send all the bytes");
                    }

                    res = 0;
                }

                n_sent += res;
            }

            return true;
        }

        bool client::CheckForReply(const u64 expected_id, ByteArray& data)
        {
            CCSLock lock(&RepliesMutex, __FILE__, __LINE__);
            
            tReplyMap::iterator it = Replies.find(expected_id);
            if (it != Replies.end())
            {
                data = it->second.second;
                Replies.erase(it);
                return true;
            }

            return false;
        }

        bool client::GetReply(const u64 expected_id, ByteArray& data)
        {
            while (Connected && !Terminate)
            {
                if (CheckForReply(expected_id, data)) return true;
                ThreadSleep(5);
            }

            return false;
        }

        const ByteArray* client::ForgeRequest(int command, u64 packet_id, const ByteArray& data)
        {
            const size_t packet_size = data.size() + HEADER_SIZE;

            ByteArray* packet = new ByteArray();
            packet->try_resize(packet_size);

            (*packet)[0] = ePacketType_Request;
            *((u16*)(&(*packet)[1])) = swap16(command);
            *((u32*)(&(*packet)[3]))  = swap32(packet_size);
            *((u64*)(&(*packet)[7])) = swap64(packet_id);

            memcpy(packet->begin() + HEADER_SIZE, data.begin(), data.size());

            return packet;
        }

        bool client::ForgeSend(int command, u64 packet_id, const ByteArray& data)
        {
            const ByteArray* sent_packet = ForgeRequest(command, packet_id, data);
            {
                CCSLock lock(&PacketSendMutex, __FILE__, __LINE__);
                Packets.push_back(sent_packet);
            }

            return true;
        }

        bool client::ForgeSendReply(int command, u64 packet_id, const ByteArray& data, ByteArray& reply_data)
        {
            if (!ForgeSend(command, packet_id, data))
                return false;

            if (!GetReply(packet_id, reply_data))
                return false;

            return true;
        }

        bool client::HandleOutput()
        {
            CRawVector<const ByteArray*> packets;
            {
                CCSLock lock(&PacketSendMutex, __FILE__, __LINE__);
                packets = Packets;
                Packets.try_resize(0);
            }

            bool fail = false;
            for (const ByteArray** it = packets.begin(); it != packets.end(); ++it)
            {
                const ByteArray* b = *it;

                if (fail)
                {
                    delete b;
                    continue;
                }

                if (!SendPacket(*b)) fail = true;
                
                delete b;
            }

            return !fail;
        }

        bool client::HandleInput()
        {
            u8 header[HEADER_SIZE];
            int res = ReadExactly(header, HEADER_SIZE);
            switch (res)
            {
                case eRecv_NoConnection: return ErrorAndDisconnect("Disconnected");
                case eRecv_Fatal:
                case eRecv_Timeout:
                    return ErrorAndDisconnect("Failed to read a packet header on socket");
                case eRecv_NoData: return true;
                case eRecv_Success: break;
                case eRecv_Terminate: return ErrorAndDisconnect("Recv was forcefully aborted");
            }

            const u8 packet_type = header[0];
            const u16 command = swap16(*(u16*)(header + 1));
            const u32 packet_size = swap32(*(u32*)(header + 3));
            const u64 packet_id = swap64(*(u64*)(header + 7));

            if (packet_size < HEADER_SIZE)
                return ErrorAndDisconnect("Invalid packet size");
            
            ByteArray data;
            if (packet_size > HEADER_SIZE)
            {
                const u32 data_size = packet_size - HEADER_SIZE;
                data.try_resize(data_size);
                if (ReadExactly((u8*)data.begin(), data_size) != eRecv_Success)
                    return ErrorAndDisconnect("Failed to receive a whole packet");
            }

            switch (packet_type)
            {
                case ePacketType_Request: return ErrorAndDisconnect("Client shouldn't receive packet requests!");
                case ePacketType_Reply:
                {
                    if (data.empty())
                        return ErrorAndDisconnect("Reply packet without result");

                    switch (command)
                    {
                        case eCommandType_ResetState: break;
                        case eCommandType_RequestSignalingInfos:
                        {
                            u32 conn_id;
                            {
                                CCSLock lock(&SignalingRequestMutex, __FILE__, __LINE__);
                                
                                tSignalInfoRequestsMap::iterator it = SignalingRequests.find(packet_id);
                                conn_id = it->second;
                                SignalingRequests.erase(it);
                            }

                            MMLog("Received signaling info reply for conn_id=%08x\n", conn_id);

                            u8 err = data[0];
                            if (err != 0)
                            {
                                MMLog("error in siginfo!\n");
                                break;
                            }
                            
                            // Technically speaking as long as we're on a compatible network protocol,
                            // I don't need to actually use the flatbuffer shit, so we're hardcoding the offsets!!!
                            // Flatbuffer would be a pain to import on here anyway.
                            u16 port = swap16(*(u16*)(data.begin() + 0x17));
                            u32 addr = *(u32*)(data.begin() + 0x21);

                            MMLog("sig: %d.%d.%d.%d:%d\n", 
                                (addr >> 24) & 0xff,
                                (addr >> 16) & 0xff,
                                (addr >> 8) & 0xff,
                                (addr >> 0) & 0xff,
                                port
                            );

                            Signaling->StartSignaling(conn_id, addr, port);

                            break;
                        }
                        default:
                        {
                            CCSLock lock(&RepliesMutex, __FILE__, __LINE__);
                            Replies.insert(std::make_pair(packet_id, std::make_pair(command, data)));
                            break;
                        }
                    }

                    break;
                }
                case ePacketType_Notification:
                {
                    vec_stream vdata = vec_stream(data);
                    switch (command)
                    {
                        case eNotificationType_SignalingHelper:
                        {
                            MMLog("Got signaling helper, but we're ignoring it for now!\n");
                            break;
                        }
                        case eNotificationType_FriendPresenceChanged:
                        {
                            const MMString<char> username = vdata.get_string();
                            MMLog("Got presence update for %s!\n", username.c_str());

                            friend_online_data* f = FindFriend(username.c_str());
                            if (f == NULL) break;

                            f->CommunicationId = vdata.get_com_id();
                            f->PresenceTitle = vdata.get_string();
                            f->PresenceStatus = vdata.get_string();
                            f->PresenceComment = vdata.get_string();
                            f->PresenceData = vdata.get_rawdata();

                            NP->SendPresenceUpdate(f);

                            break;
                        }
                        case eNotificationType_FriendStatus:
                        {
                            const bool online = !!vdata.get<u8>();
                            const u64 timestamp = vdata.get<u64>();
                            const MMString<char> username = vdata.get_string();

                            MMLog("Got friend status for %s, online: %s, timestamp: 0x%llx\n", 
                                username.c_str(),
                                online ? "true" : "false",
                                timestamp
                            );

                            friend_online_data* f = FindFriend(username.c_str());
                            if (f != NULL)
                            {
                                if (timestamp < f->Timestamp) break;

                                f->Online = online;
                                f->Timestamp = timestamp;

                                NP->SendPresenceUpdate(f);
                            }

                            break;
                        }
                        default:
                        {
                            MMLog("Got unhandled notification (%08x), ignoring!\n", command);
                            break;
                        }
                    }

                    break;
                }
                case ePacketType_ServerInfo:
                {
                    if (data.size() != 4)
                        return ErrorAndDisconnect("Invalid size of ServerInfo packet");

                    ReceivedVersion = 
                        data[0] | 
                        data[1] << 8 | 
                        data[2] << 16 | 
                        data[3] << 24;

                    ReceivedServerInfo = true;

                    break;
                }
                default:
                    return ErrorAndDisconnect("Unknown packet received!");
            }

            return true;
        }

        bool client::ErrorAndDisconnect(const char* msg)
        {
            Connected = false;
            MMLog("rpcn: %s\n", msg);
            return false;
        }

        const CVector<friend_online_data>& client::GetFriends() const
        {
            return FriendData;
        }

        friend_online_data* client::FindFriend(const char* name)
        {
            for (friend_online_data* data = FriendData.begin(); data != FriendData.end(); ++data)
            {
                if (data->Name == name)
                    return data;
            }

            return NULL;
        }

        bool client::RequestSignalingInfos(u32 req_id, const MMString<char>& npid)
        {
            ByteArray data;
            data.try_resize(npid.size() + 1);
            memcpy(data.begin(), npid.c_str(), npid.size());
            data[npid.size()] = '\0';

            MMLog("forging request for %s\n", npid.c_str());
            
            return ForgeSend(eCommandType_RequestSignalingInfos, req_id, data);
        }

        void client::SetPresence(const MMString<char>& status, const ByteArray& data)
        {
            bool update = false;

            if (!status.empty())
            {
                if (status != MyPresence.Status)
                {
                    MyPresence.Status = status;
                    update = true;
                }
            }

            if (!data.empty())
            {
                if (data != MyPresence.Data)
                {
                    MyPresence.Data = data;
                    update = true;
                }
            }
            
            // if handler is null we probably dont have the communication id
            if (NPBasicEventHandler == NULL) return;


            if (!MyPresence.Advertised || update)
            {
                MyPresence.Advertised = true;
            }
        }

        bool client::Connect(const char* host)
        {
            const u16 TCP_PORT = 31313;
            const u16 UDP_PORT = 3657;

            MMLog("connect: Attempting to connect to RPCN\n");

            {
                CCSLock lock_read(&ReadMutex, __FILE__, __LINE__), 
                        lock_write(&WriteMutex, __FILE__, __LINE__);

                Disconnect();

                if (wolfSSL_Init() != WOLFSSL_SUCCESS)
                {
                    MMLog("connect: Failed to initialize wolfssl\n");
                    return false;
                }

                WSSLc = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
                if (WSSLc == NULL)
                {
                    MMLog("connect: Failed to create wolfssl context\n");
                    return false;
                }

                wolfSSL_CTX_set_verify(WSSLc, SSL_VERIFY_NONE, NULL);

                WSSLr = wolfSSL_new(WSSLc);
                if (WSSLr == NULL)
                {
                    MMLog("connect: Failed to create wolfssl object\n");
                    return false;
                }


                memset(&ServerAddr, 0, sizeof(ServerAddr));
                ServerAddr.sin_port = htons(TCP_PORT);
                ServerAddr.sin_family = AF_INET;

                hostent* hp = gethostbyname(host);
                if (hp == NULL)
                {
                    MMLog("connect: Failed to gethostbyname %s\n", host);
                    return false;
                }

                if (hp->h_addrtype != AF_INET)
                {
                    MMLog("connect: Failed to get IPv4 address for host %s\n", host);
                    return false;
                }

                memcpy(&ServerAddr.sin_addr, hp->h_addr, hp->h_length);
                memcpy(&NPServerAddr, &ServerAddr, sizeof(NPServerAddr));
                NPServerAddr.sin_port = htons(UDP_PORT);

                Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
                if (Socket == -1)
                {
                    MMLog("connect: Failed to connect to RPCN server!\n");
                    return false;
                }

                if (connect(Socket, (sockaddr*)&ServerAddr, sizeof(ServerAddr)) != 0)
                {
                    MMLog("connect: Failed to connect to RPCN server!\n");
                    return false;
                }
                
                MMLog("connect: Connection successful\n");

                int blocking_mode = 1;
                if (setsockopt(Socket, SOL_SOCKET, SO_NBIO, &blocking_mode, 4) != 0)
                {
                    MMLog("connect: Failed to set non blocking socket!\n");
                    return false;
                }

                sockaddr_in client_addr;
                socklen_t client_addr_size = sizeof(client_addr);
                if (getsockname(Socket, (sockaddr*)&client_addr, &client_addr_size) != 0)
                {
                    MMLog("connect: Failed to get the client address from the socket!\n");
                }

                LocalAddress = client_addr.sin_addr.s_addr;
                MMLog("connect: local addr %d.%d.%d.%d:%d\n", 
                    (LocalAddress >> 24) & 0xff,
                    (LocalAddress >> 16) & 0xff,
                    (LocalAddress >> 8) & 0xff,
                    (LocalAddress >> 0) & 0xff,
                    client_addr.sin_port
                );
                
                if (wolfSSL_set_fd(WSSLr, Socket) != WOLFSSL_SUCCESS)
                {
                    MMLog("connect: Failed to connect wolfssl to the socket\n");
                    return false;
                }

                int ret;
                while ((ret = wolfSSL_connect(WSSLr)) != SSL_SUCCESS)
                {
                    if (wolfSSL_want_read(WSSLr) || wolfSSL_want_write(WSSLr))
                        continue;

                    MMLog("connect: Handshake failed with RPCN Server!\n");
                    return false;
                }

                MMLog("connect: Handshake successful\n");

                if ((WSSLw = wolfSSL_write_dup(WSSLr)) == NULL)
                {
                    MMLog("connect: Failed to create write dup for SSL\n");
                    return false;
                }
            }

            Connected = true;

            while (!ReceivedServerInfo && Connected && !Terminate)
                ThreadSleep(5);

            if (!Connected || Terminate)
            {
                return false;
            }

            if (ReceivedVersion != PROTOCOL_VERSION)
            {
                MMLog("Server returned protocol version: %d, expected: %d\n", ReceivedVersion, PROTOCOL_VERSION);
                return false;
            }

            MMLog("connect: Protocol version matches\n");

            return true;
        }

        bool client::Login(const MMString<char>& npid, const MMString<char>& password, const MMString<char>& token)
        {
            if (npid.empty()) return false;
            if (password.empty()) return false;

            MMLog("attempting to login!\n");

            ByteArray data;
            for (int i = 0; i < npid.size(); ++i) data.push_back(npid.c_str()[i]);
            data.push_back(0);
            for (int i = 0; i < password.size(); ++i) data.push_back(password.c_str()[i]);
            data.push_back(0);
            for (int i = 0; i < token.size(); ++i) data.push_back(token.c_str()[i]);
            data.push_back(0);

            ByteArray packet_data;
            if (!ForgeSendReply(eCommandType_Login, RequestCounter++, data, packet_data))
                return false;

            vec_stream reply(packet_data);
            u8 error = reply.get<u8>();

            if (error != 0)
            {
                Disconnect();
                return false;
            }

            MMString<char> online_name = reply.get_string();


            MMString<char> avatar_url = reply.get_string();
            UserID = reply.get<s64>();
            
            MMLog("Logged in as %s\n", online_name.c_str());

            u32 nufriends = reply.get<u32>();
            FriendData.try_resize(nufriends);
            for (u32 i = 0; i < nufriends; ++i)
            {   
                friend_online_data& f = FriendData[i];

                f.Name = reply.get_string();
                f.Online = !!(reply.get<u8>());

                f.CommunicationId = reply.get_com_id();
                f.PresenceTitle = reply.get_string();
                f.PresenceStatus = reply.get_string();
                f.PresenceComment = reply.get_string();
                f.PresenceData = reply.get_rawdata();

                MMLog("\t%s (%s)\n", f.Name.c_str(), f.Online ? "ONLINE" : "OFFLINE");
            }

            Authentified = true;
        }

        bool client::TerminateConnection()
        {
            ByteArray packet_data;
            ByteArray data;
            if (!ForgeSendReply(eCommandType_Terminate, RequestCounter++, data, packet_data))
                return false;
        }

        void client::Disconnect()
        {
            if (WSSLr)
            {
                wolfSSL_free(WSSLr);
                WSSLr = NULL;
            }

            if (WSSLw)
            {
                wolfSSL_free(WSSLw);
                WSSLw = NULL;
            }

            if (WSSLc)
            {
                wolfSSL_CTX_free(WSSLc);
                WSSLc = NULL;
            }

            wolfSSL_Cleanup();

            if (Socket)
            {
                shutdown(Socket, SHUT_RDWR);
                socketclose(Socket);
                Socket = 0;
            }

            Connected = false;
            Authentified = false;
            ReceivedServerInfo = false;
        }

        client* Client;
    }
}