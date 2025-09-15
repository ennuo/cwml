#pragma once

#include <network/NetworkUtilsNP.h>
#include <Slot.h>
#include <vector.h>

enum EOnlineStatus {
    E_OFFLINE,
    E_IN_OTHER_GAME,
    E_IN_LITTLE_BIG_PLANET,
    E_STATUS_INVALID
};

enum EMatchmakingMood {
    E_MOOD_ALONE,
    E_MOOD_FRIENDS,
#ifndef LBP1
    E_MOOD_STRANGERS,
#endif
    E_MOOD_EVERYONE,

    E_MOOD_COUNT,
    INVALID_MOOD = E_MOOD_COUNT
};

enum EJoinAllowedByProgression {
    E_STILL_TOO_EARLY_IN_PROGRESSION_TO_JOIN,
    E_HAS_PROGRESSED_FAR_ENOUGH_SO_YOU_CAN_JOIN,
    E_JOIN_ALLOWED_BY_PROGRESSION_COUNT,
    INVALID_JOIN_ALLOWED_BY_PROGRESSION
};

class CFriendData {
public:
    NetworkPlayerID PlayerID;
    NetworkAvatarURL AvatarURL;
    EOnlineStatus OnlineStatus;
    EMatchmakingMood Mood;
    EJoinAllowedByProgression JoinAllowedByProgression;
    NetworkPlayerID PartyLeaderID;
    NetworkPlayerID GameLeaderID;
    CSlotID LevelSlotID;
    s32 InviteNotificationID;
    u8 EstimatedNumSlotsUsed;
    float WeHaveInvitedThisPlayerToJoinUs;
    float ThisPlayHasInvitedUsToJoinThem;
    bool EditMode;
    bool InTutorial;
    bool WrongVersion;
    float LastTimeWeUpdatedThisPlayersPresence;
    char PresenceString[100];
};

class CNetworkFriendsManager {
enum EPrepareStage {
    E_PREPARE_STAGE_INIT,
    E_PREPARE_STAGE_GET_BLOCK_LIST,
    E_PREPARE_STAGE_FULLY_CONNECTED
};
public:
    EPrepareStage PrepareStage;
    u32 NumEventsToRetrieve;
    ByteArray EventDataPool;
    CRawVector<CFriendData> FriendData;
    bool HavePresenceForAllFriends;
};

