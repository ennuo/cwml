#pragma once

#include "Part.h"

class PPhysicsTweak : public CPart {
public:
    inline int& GetConfiguration() { return *(int*)(((char*)this) + 0xd4); }
    inline bool& GetWaitingToMove() { return *(bool*)(((char*)this) + 0x94); }
    inline float& GetLastKnownActivation() { return *(float*)(((char*)this) + 0x90); }
    inline int& GetZBehaviour() { return *(int*)(((char*)this) + 0x68); }
    inline bool& GetCanPush() { return *(bool*)(((char*)this) + 0x64); }
    inline float& GetMaximumMass() { return *(float*)(((char*)this) + 0x60); }
};
