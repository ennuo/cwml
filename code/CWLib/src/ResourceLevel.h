#pragma once

#include "thing.h"
#include "Resource.h"
#ifndef LBP1
#include "InventoryItem.h"
#endif
#include "network/NetworkUtilsNP.h"

class CPlayerRecord {
protected:
    NetworkPlayerID PlayerIDs[32];
    s32 PlayerNumbers[32];
    u32 Offset;
};

class RLevel : public CResource {
public:
    inline PWorld* GetWorld()
    {
        if (WorldThing.GetThing() != NULL)
            return WorldThing->GetPWorld();
        return NULL;
    }
public:
#ifndef LBP1
    CVector<CInventoryItem> TutorialInventory;
    CVector<CCachedInventoryData> TutorialInventoryData;
#endif
    CThingPtr WorldThing;
    CPlayerRecord PlayerRecord;
};
