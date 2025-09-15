#include "ThingPartContainer.h"

#ifndef LBP1

enum EPartCache {
    PART_CACHE_INVALID = -1,
    PART_CACHE_JOINT = 0,
    PART_CACHE_WORLD,
    PART_CACHE_TRIGGER,
    PART_CACHE_YELLOW_HEAD,
    PART_CACHE_AUDIO_WORLD,
    PART_CACHE_GENERATED_MESH,
    PART_CACHE_SPRITE_LIGHT,
    PART_CACHE_SCRIPT_NAME,
    PART_CACHE_CREATURE,
    PART_CACHE_STICKERS,
    PART_CACHE_DECORATIONS,
    PART_CACHE_SCRIPT,
    PART_CACHE_EMITTER,
    PART_CACHE_COSTUME,
    PART_CACHE_CAMERA_TWEAK,
    PART_CACHE_SWITCH,
    PART_CACHE_SWITCH_KEY,
    PART_CACHE_GAMEPLAY_DATA,
    PART_CACHE_ENEMY,
    PART_CACHE_GROUP,
    PART_CACHE_PHYSICS_TWEAK,
    PART_CACHE_NPC,
    PART_CACHE_SWITCH_INPUT,
    PART_CACHE_MICROCHIP,
    PART_CACHE_MATERIAL_TWEAK,
#if defined(LBP3) || defined(HAS_CROSS_CONTROLLER)
    PART_CACHE_ANIMATION_TWEAK,
    PART_CACHE_WIND_TWEAK,
    PART_CACHE_ATMOSPHERIC_TWEAK,
    PART_CACHE_STREAMING_DATA,
#endif
};

bool CPartContainer::HasPart(EPartType type) const 
{ 
    return Flags & (1ull << type); 
}

CPart* CPartContainer::GetPart(EPartType type) const
{
    // Ideally this should mostly be optimized out
    // due to the switch case and constants, but who knows.
    #if defined(LBP3) || defined(HAS_CROSS_CONTROLLER)
        #define HAS_CACHED_PART(cache) ((CacheFlags & (1 << cache)))
    #else
        #define HAS_CACHED_PART(cache) (Flags & (1ull << (PART_TYPE_SIZE + cache)))
    #endif

    #define PART_MACRO(name, type, cache) \
    case type: \
    { \
        if (type == PART_TYPE_BODY) return PBody; \
        if (type == PART_TYPE_RENDER_MESH) return PRenderMesh; \
        if (type == PART_TYPE_POS) return PPos; \
        if (type == PART_TYPE_SHAPE) return PShape; \
        if (cache != PART_CACHE_INVALID) \
        { \
            if (HAS_CACHED_PART(cache)) \
                return CachedParts[(CachedPartIndex >> (cache << 1)) & 3]; \
        } \
        if (Flags & (1ull << type)) \
            return Parts[type]; \
        return NULL; \
    }

    switch (type) 
    {
        #include "PartList.h"
    }

    return NULL;

    #undef PART_MACRO
    #undef HAS_CACHED_PART
}

#endif // LBP1