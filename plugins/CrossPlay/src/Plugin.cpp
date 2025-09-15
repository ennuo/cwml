#include <cwml/cwml.h>
#include <DebugLog.h>

#include <CrossPlay.h>
#include <network/NetworkManager.h>
#include <rpcn/Client.h>
#include <rpcn/NPHandler.h>

#include <Ib/Assembly/PowerPC.h>

using namespace crossplay;

enum EDivergenceCheck
{
    eDivergenceCheck_Default,
    eDivergenceCheck_Disabled,
    eDivergenceCheck_Lossy
};

cwml::ConfigOption<s32> gDivergenceCheck("Divergence Check Mode", "divergencecheck", eDivergenceCheck_Disabled);

void DoDivergenceHack()
{
    if (gDivergenceCheck == eDivergenceCheck_Disabled)
        return;
    
}

bool LookupFriendPlayerIDFromName(const char* friend_player_handle, NetworkPlayerID& player_id)
{
    const bool FORCE_CREATE_FRIEND_ENTRY = false;

    MMLog("LOOKING UP %s\n", friend_player_handle);
    CRawVector<CFriendData>& friends = gNetworkManager.FriendsManager.FriendData;
    const CVector<rpcn::friend_online_data>& rpcn_friends = rpcn::Client->GetFriends();
    for (int i = 0; i < rpcn_friends.size(); ++i)
    {
        const rpcn::friend_online_data& f = rpcn_friends[i];
        bool found = false;
        for (int j = 0; j < friends.size(); ++j)
        {
            CFriendData& friend_data = friends[j];
            if (strcmp(GetPlayerName(friend_data.PlayerID), f.Name.c_str()) != 0) continue;
            found = true;
            break;
        }

        if (found) continue;

        MMLog("adding %s to friends list\n", f.Name.c_str());
        
        CFriendData data;
        memset(&data, 0, sizeof(CFriendData));
        strcpy(data.PlayerID.handle.data, f.Name.c_str());
        data.OnlineStatus = E_IN_LITTLE_BIG_PLANET;
        data.Mood = E_MOOD_EVERYONE;
        data.JoinAllowedByProgression = E_JOIN_ALLOWED_BY_PROGRESSION_COUNT;

        friends.push_back(data);
    }

    for (int i = 0; i < friends.size(); ++i)
    {
        CFriendData& f = friends[i];
        if (f.OnlineStatus == E_STATUS_INVALID) continue;
        if (strcmp(GetPlayerName(f.PlayerID), friend_player_handle) == 0)
        {
            player_id = f.PlayerID;
            return true;
        }
    }

    if (FORCE_CREATE_FRIEND_ENTRY)
    {
        CFriendData data;
        memset(&data, 0, sizeof(CFriendData));
        strcpy(data.PlayerID.handle.data, friend_player_handle);
        data.OnlineStatus = E_IN_LITTLE_BIG_PLANET;
        data.Mood = E_MOOD_EVERYONE;
        data.JoinAllowedByProgression = E_JOIN_ALLOWED_BY_PROGRESSION_COUNT;

        friends.push_back(data);
        player_id = data.PlayerID;
        return true;
    }

    return false;
}




class CrossPlay : public cwml::Plugin {
public:
    const char* GetName() const { return "CrossPlay"; }

    bool OnAttach() 
    { 
        MMLog("crossplay: attaching hooks\n");

        if (gDivergenceCheck != eDivergenceCheck_Default)
            Ib_ReplacePort(CheckDivergenceData, DoDivergenceHack);

        if (Ib::IsEmulator()) return true;

        Ib_ReplacePort(sceNpSignalingActivateConnection, signaling::ActivateConnection);
        Ib_ReplacePort(sceNpSignalingCreateCtx, signaling::CreateContext);
        Ib_ReplacePort(sceNpSignalingDeactivateConnection, signaling::DeactivateConnection);
        Ib_ReplacePort(sceNpSignalingGetConnectionInfo, signaling::GetConnectionInfo);
        Ib_ReplacePort(sceNpSignalingGetConnectionStatus, signaling::GetConnectionStatus);
        Ib_ReplacePort(cellSysutilCheckCallback, CheckCallbacks);
        Ib_ReplacePort(sceNpBasicRegisterContextSensitiveHandler, npbasic::RegisterContextSensitiveHandler);
        Ib_ReplacePort(sceNpBasicGetEvent, npbasic::GetBasicEvent);
        
        // Ib_ReplacePort(PartyManagement_LookupFriendPlayerIDFromName, LookupFriendPlayerIDFromName);
        
        Ib_ReplacePort(sendto, OnSendP2PPacket);
        Ib_ReplacePort(recvfrom, OnReceiveP2PPacket);

        // u32 WEBTERNATE_PORT = LI(4, 0x5742);
        // Ib_Write(0x00415574, &WEBTERNATE_PORT, sizeof(u32));

        return true; 
    }

    bool OnOpen(bool was_reloaded)
    {
        // Don't load any of the extra shit on RPCS3
        if (Ib::IsEmulator()) return true;

        if (was_reloaded)
            crossplay::rpcn::StartupTimeout = 1000;

        crossplay::Initialise();

        return true;
    }

    void OnClose()
    {
        MMLog("crossplay: close\n");
        crossplay::Close();
    }
};

REGISTER_PLUGIN(CrossPlay);
