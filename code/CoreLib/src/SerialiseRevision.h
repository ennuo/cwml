#pragma once

#include <BranchDefines.h>

const u32 gFormatRevision = 0x272;
const u32 gFormatBranchDescription = 0x4c440017;

enum
{
    SR_INITIAL,

    SR_SLOT_GROUPS = 0x134, // added group links to slots
    SR_SECONDARY_RIM_COLOUR = 0x138, // added secondary rim color to level settings
    SR_SCRIPT_VM_TYPE = 0x145, // added fish type to script instance field layouts
    SR_SLOT_AUTHOR = 0x13b, // added author name to slot
    SR_JOINT_SOUNDS = 0x16a, // added sound enum to pjoint
    SR_LEVEL_KEYS = 0x177, // inventory keys can unlock levels
    SR_COMPRESSED_RESOURCES = 0x189, // resources can be optionally compressed
    SR_SLOT_LEVEL_TYPE = 0x1d4, // added level type for developer slots
    SR_SCRIPT_MODIFIERS = 0x1e5, // added modifiers to scripts
    SR_SHARED_SCRIPTS = 0x1ec, // scripts now use shared pools for all types
    SR_THING_STAMPING = 0x21b, // keep track of whether a thing is being stamped
    SR_INVENTORY_TIMESTAMPS = 0x222, // keep track of the date an item was added to inventory
    SR_INVENTORY_SHAREABLE = 0x223, // keep track of whether an item is shareable in inventory
    SR_PLAN_GUID = 0x254, // keep track of the plan a thing comes from
    SR_PROCEDURAL_PLAN_GUID = 0x258, // keep track of the plan a generated mesh comes from
    SR_COMPRESSED_DECAL_COLOUR = 0x260, // use rgb565 for decals instead of rgba32
    SR_DLC_CATEGORY = 0x264, // added category id to dlc files
    SR_PICKUP_GROUP_MEMBERS = 0x26e, // added flag for whether entire group should be picked up
    SR_WATER_LEVEL = 0x26f, // customizable water level
    SR_WATER_MAGNITUDE = 0x270, // customizable water magnitude
    SR_BRANCH_DESCRIPTION = 0x271, // branchable resource revisions
    SR_CREATURE_AIRTIME = 0x272,

    SR_LATEST_PLUS_ONE
};

#define SR_LATEST (SR_LATEST_PLUS_ONE - 1)
