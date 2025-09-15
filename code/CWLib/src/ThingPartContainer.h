#pragma once

#include <Part.h>
#include <PartTypeEnum.h>
#include <PartList.h>

#ifndef LBP1

#define PART_CACHE_SIZE (4)

class CPartContainer {
public:
    bool HasPart(EPartType type) const;
    CPart* GetPart(EPartType type) const;
private:
    u64 Flags;
    u64 CachedPartIndex;
#if defined(LBP3) || defined(HAS_CROSS_CONTROLLER)
    u32 CacheFlags;
#endif
    CPart* PBody;
    CPart* PRenderMesh;
    CPart* PPos;
    CPart* PShape;
#if !defined(LBP3) && !defined(HAS_CROSS_CONTROLLER)
    CPart* _;
#endif
    CPart** Parts;
    CPart* CachedParts[PART_CACHE_SIZE];
};

#endif