#pragma once

enum EPartType
{
#define PART_MACRO(name, type, cache) type,
    #include <PartList.h>
#undef PART_MACRO
    PART_TYPE_SIZE
};
