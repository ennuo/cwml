#pragma once

#include "ReflectionVisitable.h"
#include "ResourceDescriptor.h"
#include "Slot.h"
#include "GuidHash.h"
#include "Colour.h"
#include "MMString.h"
#include "vector.h"
#include "network/NetworkUtilsNP.h"

class RPlan;

struct CCachedUV {
    u8 U0;
    u8 U1;
    u8 V0;
    u8 V1;
};


class CCachedInventoryData {
public:
    u32 Category;
    u32 Location;
    u32 CachedToolType;
    CCachedUV CachedUVs;
};

class CInventoryItemDetails : public CDependencyWalkable {
#ifdef LBP1
public:
    virtual void SetIcon(RTexture* icon);
public:
    CGUID HighlightSound;
    CSlotID LevelUnlockSlotID;
    u32 LocationIndex;
    u32 CategoryIndex;
    u32 PrimaryIndex;
    u32 LastUsed;
    u32 NumUses;
    u32 Pad;
    /* CalendarTime */ u64 DateAdded;
    int FluffCost;
    c32 Colour;
    u32 Type;
    u32 SubType;
    u32 ToolType;
    NetworkPlayerID Creator;
    bool AllowEmit;
    bool Shareable;
    bool Copyright;
    u32 NameTranslationTag;
    u32 DescTranslationTag;
    MMString<tchar_t> UserCreatedName;
    MMString<tchar_t> UserCreatedDescription;
    CVector<MMString<tchar_t> > EditorList;
    void* Icon;
    // dont need these structs right now
    void* PhotoData;
    void* EyetoyData;
#else
public:
    u64 DateAdded;
    CSlotID LevelUnlockSlotID;
    CGUID HighlightSound;
    c32 Colour;
    u32 Type;
    u32 SubType;
    u32 NameTranslationTag;
    u32 DescTranslationTag;
    void* CreationHistory;
    void* Icon;
    void* UserCreatedDetails;
    void* PhotoData;
    void* EyetoyData;
    void* Creator;
    u16 LocationIndex;
    u16 CategoryIndex;
    u16 PrimaryIndex;
    u8 ToolType;
    u8 Flags;
#endif
};

class CInventoryItem {
public:
    inline CInventoryItem() : Plan(), Details(), UID(), Flags() {}
public:
#ifdef LBP1
    CResourceDescriptor<RPlan> Plan;
#else
    CPlanDescriptor Plan;
#endif
    CInventoryItemDetails Details;
    u32 UID;
#ifdef LBP1
    u32 TutorialLevel;
    u32 TutorialVideo;
#endif 
protected:
    u32 Flags;
public:
#ifndef LBP1
    u32 UserCategoryIndex;
#endif
};


