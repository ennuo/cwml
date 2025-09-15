#pragma once

#include <rpcn/Types.h>

#include <thread.h>
#include <MMString.h>
#include <vector.h>
#include <network/NetworkUtilsNP.h>
#include <CritSec.h>

#include <map>
#include <set>

#include <wolfssl/ssl.h>
#include <cwml/Config.h>

namespace crossplay
{
    namespace rpcn
    {
        const int PROTOCOL_VERSION = 26;
        const int HEADER_SIZE = 15;
        const int TIMEOUT_INTERVAL = 50;
        const int TIMEOUT = 10000;

        extern int MatchReqID;
        extern int StartupTimeout;

        extern cwml::ConfigOption<const char*> gHost;
        extern cwml::ConfigOption<const char*> gNPID;
        extern cwml::ConfigOption<const char*> gPassword;
        extern cwml::ConfigOption<const char*> gToken;


        typedef std::map<u64, std::pair<int, ByteArray> > tReplyMap;
        typedef std::map<u32, u32> tSignalInfoRequestsMap;
        typedef std::set<in_addr_t> tAddressSet;

        u32 GetRequestID(u16 high);

        extern tSignalInfoRequestsMap SignalingRequests;
        extern tAddressSet Addresses;
        extern CCriticalSec SignalingRequestMutex;
        
        class client {
        public:
            client();
            ~client();
        private:
            static void ReaderThreadStatic(u64 args);
            static void WriterThreadStatic(u64 args);
            static void MainThreadStatic(u64 args);
        private:
            void ReaderThread();
            void WriterThread();
            void MainThread();
        private:
            int ReadExactly(u8* buf, size_t n);
        private:
            bool SendPacket(const ByteArray& packet);
            bool CheckForReply(const u64 expected_id, ByteArray& data);
            bool GetReply(const u64 expected_id, ByteArray& data);
            const ByteArray* ForgeRequest(int command, u64 packet_id, const ByteArray& data);
            bool ForgeSend(int command, u64 packet_id, const ByteArray& data);
            bool ForgeSendReply(int command, u64 packet_id, const ByteArray& data, ByteArray& reply_data);
            bool HandleOutput();
            bool HandleInput();
            bool ErrorAndDisconnect(const char* msg);
        private:
            bool Connect(const char* host);
            bool Login(const MMString<char>& npid, const MMString<char>& password, const MMString<char>& token);
            bool TerminateConnection();
            void Disconnect();
        public:
            const CVector<friend_online_data>& GetFriends() const;
            friend_online_data* FindFriend(const char* name);
        public:
            bool RequestSignalingInfos(u32 req_id, const MMString<char>& npid);
            void SetPresence(const MMString<char>& status, const ByteArray& data);
        private:
            int mState;
            bool Connected;
            bool Authentified;
            bool Terminate;
            bool WantConnection;
            bool WantAuthentication;
            bool ReceivedServerInfo;
            u32 ReceivedVersion;
            u32 BindingAddress;
            u32 LocalAddress;
            u32 PublicAddress;
            u32 PublicPort;
            WOLFSSL_CTX* WSSLc;
            WOLFSSL* WSSLr;
            WOLFSSL* WSSLw;
            sockaddr_in ServerAddr;
            sockaddr_in NPServerAddr;
            int Socket;
            s64 UserID;
            THREAD ReadThread;
            THREAD WriteThread;
            THREAD Thread;
            CCriticalSec ReadMutex;
            CCriticalSec WriteMutex;
            CCriticalSec PacketSendMutex;
            CCriticalSec RepliesMutex;
            u64 RequestCounter;
            CRawVector<const ByteArray*> Packets;
            CVector<friend_online_data> FriendData;
            tReplyMap Replies;

            struct
            {
                MMString<char> Title;
                MMString<char> Status;
                ByteArray Data;
                bool Advertised;
            } MyPresence;
        };

        extern client* Client;
    }
}