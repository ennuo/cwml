#pragma once

#include "Part.h"
#include "PartSwitch.h"

class PEmitter : public CPart {
struct Forked {
#ifdef LBP1
    float LastUpdateFrame;
#else
    float ParentRelativeRotation;
    float SpeedScaleStartFrame;
    float SpeedScaleDeltaFrames;
    CSwitchSignal SpeedScale;
#endif
};
public:
    Forked Game;
    Forked Rend;
    Forked* Fork;
#ifdef LBP1
    v4 PosVel;
    float LinearVel;
#endif
    float AngVel;
    u32 Frequency;
    u32 Phase;
#ifdef LBP1
    u32 Lifetime;
    CP<RPlan> Plan;
    u32 MaxEmitted;
    u32 CurrentEmitted;
    u32 MaxEmittedAtOnce;
    float ModStartFrame;
    float ModDeltaFrames;
    CSwitchSignal ModScale;
    bool ModScaleActive;
#else
    v4 ParentRelativeOffset;
    u32 Lifetime;
    u32 MaxEmitted;
    u32 CurrentEmitted;
    u32 MaxEmittedAtOnce;
    s32 Behaviour;
    CP<RPlan> Ref;
#endif
};