#pragma once

class CReflectionVisitable {
public:
    u32 VisitationStamp;
    void* Visited;
};

class CDependencyWalkable {
public:
#ifdef LBP1
    inline CDependencyWalkable() {
        DependencyWalkStamp = -1;
#ifndef NO_LIMITS_STAMP
        LimitsDependencyWalkStamp = -1;
#endif
    }

    u32 DependencyWalkStamp; 
#ifndef NO_LIMITS_STAMP
    u32 LimitsDependencyWalkStamp;
#endif
#endif // LBP1
};
