#pragma once

#ifndef LBP1

enum
{
    MEM_MAIN,
    MEM_FMOD,
    MEM_DEBUG
};

struct SBucketDef {
    const char* Name;
    u32 Size;
    u32 Type;
};

extern SBucketDef gBucketDefs[];

#endif // LBP1