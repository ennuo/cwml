#pragma once

#include "vector.h"
#include "GuidHash.h"
#include "SlotID.h"
#include "MMString.h"
#include "Resource.h"
#include "ResourceDescriptor.h"
#include "network/NetworkUtilsNP.h"

class CCollectabubble {
public:
    CResourceDescriptor<RPlan> Plan;
    u32 Count;
};

class CLabel {
public:
    u32 Key;
    u32 Order;
};

class CSlot {
public:
    inline const CSlotID& ID() const { return SlotID; }
    const tchar_t* GetAuthorName() const { return AuthorName.c_str(); }
public:
    CSlotID SlotID;
    CResourceDescriptor<RLevel> Level;
    CResourceDescriptor<RTexture> Icon;
#ifndef LBP1
    CResourceDescriptor<RLevel> PlanetDecorations;
#ifdef HAS_MOVE
    bool ShowOnPlanet;
#endif // HAS_MOVE
#endif // LBP1
    v4 Location;
    NetworkOnlineID AuthorID;
    MMString<tchar_t> AuthorName;
    MMString<char> TranslationTag;
    CSlotID PrimaryLinkLevel;
    CSlotID Group;
    bool InitiallyLocked;
    u8 Shareable;
#ifndef LBP1
    bool IsSubLevel;
#endif // LBP1
    CGUID BackgroundGUID;
    u32 DeveloperLevelType;
#ifndef LBP1
#ifdef HAS_MOVE
    u8 LivesOverride;
    u8 MinPlayers;
    u8 MaxPlayers;
    bool MoveRecommended;
#endif // HAS_MOVE
#ifdef HAS_CROSS_CONTROLLER
    bool CrossControl;
#endif // HAS_CROSS_CONTROLLER
    CBaseVector<CLabel> AuthorLabels;
    CBaseVector<CCollectabubble> CollectabubblesRequired;
    CBaseVector<CCollectabubble> CollectabubblesContained;
#endif // LBP1
    MMString<tchar_t> Name;
    MMString<tchar_t> Description;
};

typedef CVector<CSlot> V_Slot;
class RSlotList : public CResource {
public:
    V_Slot Slots;
};
