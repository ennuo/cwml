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
        LimitsDependencyWalkStamp = -1;
    }

    u32 DependencyWalkStamp; 
    u32 LimitsDependencyWalkStamp;
#endif
};
