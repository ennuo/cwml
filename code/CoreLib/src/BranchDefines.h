#pragma once

const u32 gLeerdammerFormatRevision = 0x272;
const u32 gLeerdammerBranchDescription = 0x4c440017;

struct BranchDefine {
    const char* Name;
    u32 BranchFormatRevision;
    u32 BranchDescription;
};

extern BranchDefine gBranchDefines[];