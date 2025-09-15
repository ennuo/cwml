#pragma once

#include <vector.h>
#include <MMString.h>

#include <network/NetworkUtilsNP.h>
#include <np.h>
#include <sys/socket.h>
#include <Endian.h>


namespace crossplay 
{
    const size_t COMMUNICATION_ID_COMID_COMPONENT_SIZE = 9;
    const size_t COMMUNICATION_ID_SUBID_COMPONENT_SIZE = 2;
    const size_t COMMUNICATION_ID_SIZE = COMMUNICATION_ID_COMID_COMPONENT_SIZE + COMMUNICATION_ID_SUBID_COMPONENT_SIZE + 1;

    const int VPORT_0_HEADER_SIZE = sizeof(u16) + sizeof(u8);

    enum VPORT_0_SUBSET
    {
        eSubset_RPCN      = 0,
        eSubset_Signaling = 1,
    };

    namespace rpcn 
    {

        struct friend_online_data
        {
            MMString<char> Name;
            u64 Timestamp;
            SceNpCommunicationId CommunicationId;
            MMString<char> PresenceTitle;
            MMString<char> PresenceStatus;
            MMString<char> PresenceComment;
            ByteArray PresenceData;
            bool Online;
        };

        enum
        {
            eRequestId_Misc = 0x3333,
            eRequestId_Score = 0x3334,
            eRequestId_Tus = 0x3335,
            eRequestId_Gui = 0x3336
        };

        enum
        {
            eRecv_NoConnection,
            eRecv_Fatal,
            eRecv_Timeout,
            eRecv_NoData,
            eRecv_Success,
            eRecv_Terminate
        };

        enum
        {
            eCommandType_Login,
            eCommandType_Terminate,
            eCommandType_Create,
            eCommandType_SendToken,
            eCommandType_SendResetToken,
            eCommandType_ResetPassword,
            eCommandType_ResetState,
            eCommandType_AddFriend,
            eCommandType_RemoveFriend,
            eCommandType_AddBlock,
            eCommandType_RemoveBlock,
            eCommandType_GetServerList,
            eCommandType_GetWorldList,
            eCommandType_CreateRoom,
            eCommandType_JoinRoom,
            eCommandType_LeaveRoom,
            eCommandType_SearchRoom,
            eCommandType_GetRoomDataExternalList,
            eCommandType_SetRoomDataExternal,
            eCommandType_GetRoomDataInternal,
            eCommandType_SetRoomDataInternal,
            eCommandType_GetRoomMemberDataInternal,
            eCommandType_SetRoomMemberDataInternal,
            eCommandType_SetUserInfo,
            eCommandType_PingRoomOwner,
            eCommandType_SendRoomMessage,
            eCommandType_RequestSignalingInfos,
            eCommandType_RequestTicket,
            eCommandType_SendMessage,
            eCommandType_GetBoardInfos,
            eCommandType_RecordScore,
            eCommandType_RecordScoreData,
            eCommandType_GetScoreData,
            eCommandType_GetScoreRange,
            eCommandType_GetScoreFriends,
            eCommandType_GetScoreNpid,
            eCommandType_GetNetworkTime,
            eCommandType_TusSetMultiSlotVariable,
            eCommandType_TusGetMultiSlotVariable,
            eCommandType_TusGetMultiUserVariable,
            eCommandType_TusGetFriendsVariable,
            eCommandType_TusAddAndGetVariable,
            eCommandType_TusTryAndSetVariable,
            eCommandType_TusDeleteMultiSlotVariable,
            eCommandType_TusSetData,
            eCommandType_TusGetData,
            eCommandType_TusGetMultiSlotDataStatus,
            eCommandType_TusGetMultiUserDataStatus,
            eCommandType_TusGetFriendsDataStatus,
            eCommandType_TusDeleteMultiSlotData,
            eCommandType_SetPresence,
            eCommandType_CreateRoomGUI,
            eCommandType_JoinRoomGUI,
            eCommandType_LeaveRoomGUI,
            eCommandType_GetRoomListGUI,
            eCommandType_SetRoomSearchFlagGUI,
            eCommandType_GetRoomSearchFlagGUI,
            eCommandType_SetRoomInfoGUI,
            eCommandType_GetRoomInfoGUI,
            eCommandType_QuickMatchGUI,
            eCommandType_SearchJoinRoomGUI
        };

        enum
        {
            eNotificationType_UserJoinedRoom,
            eNotificationType_UserLeftRoom,
            eNotificationType_RoomDestroyed,
            eNotificationType_UpdatedRoomDataInternal,
            eNotificationType_UpdatedRoomMemberDataInternal,
            eNotificationType_FriendQuery,  // Other user sent a friend request
            eNotificationType_FriendNew,    // Add a friend to the friendlist(either accepted a friend request or friend accepted it)
            eNotificationType_FriendLost,   // Remove friend from the friendlist(user removed friend or friend removed friend)
            eNotificationType_FriendStatus, // Set status of friend to Offline or Online
            eNotificationType_RoomMessageReceived,
            eNotificationType_MessageReceived,
            eNotificationType_FriendPresenceChanged,
            eNotificationType_SignalingHelper,
            eNotificationType_MemberJoinedRoomGUI,
            eNotificationType_MemberLeftRoomGUI,
            eNotificationType_RoomDisappearedGUI,
            eNotificationType_RoomOwnerChangedGUI,
            eNotificationType_UserKickedGUI,
            eNotificationType_QuickMatchCompleteGUI
        };

        enum
        {
            ePacketType_Request,
            ePacketType_Reply,
            ePacketType_Notification,
            ePacketType_ServerInfo
        };
    }
}