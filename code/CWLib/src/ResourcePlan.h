#pragma once

#include "Resource.h"
#include "InventoryItem.h"

#include "vector.h"
#include "refcount.h"
#include "thing.h"

class RTexture;

Ib_DeclarePort(CopyOver, void, CThing* thing_to, RPlan* plan, bool enforce_world_pos, CVector<CThingPtr>& things_out, bool remap_uids, void*);

class CPlanDetails : public CInventoryItemDetails {
public:
    inline u32 GetLocation() const { return Location; }
    inline u32 GetCategory() const { return Category; }
    inline void SetLocation(u32 location) { Location = location; }
    inline void SetCategory(u32 category) { Category = category; }
protected:
    u32 Location;
    u32 Category;
#ifdef LBP1
    void* PinnedIcon;    
    //CP<RTexture> PinnedIcon;
#endif
};

class RPlan : public CResource {
public:
    static CThing* MakeClone(RPlan* plan, PWorld* world, const NetworkPlayerID& default_creator, bool remap_uids);
    static CP<RPlan> CopyPlan(RPlan* plan, CThing* thing);
#ifdef LBP1
public:
    inline const CThing* GetAssignedThing() { return AssignedThing; }
protected:
    CThing* AssignedThing;
#else
private:
    char mPad[0x10];
#endif
public:
    u32 Revision;
    u32 BranchDescription;
    u8 CompressionFlags;
    ByteArray ThingData;
    CPlanDetails InventoryData;
};
