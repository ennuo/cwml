#pragma once

#include <ReflectionVisitable.h>
#include <GuidHash.h>
#include <vector.h>

#include <Part.h>
#ifndef LBP1
    #include <ThingPartContainer.h>
#endif

class CThing;

class CThingPtr {
public:
    CThingPtr();
    CThingPtr(CThing* thing);
    ~CThingPtr();
    void Unset();
    void Set(CThing* thing);
    CThingPtr& operator=(CThingPtr const& rhs);
    CThingPtr& operator=(CThing* rhs);
    inline operator CThing*() const { return Thing; }
    inline CThing* operator->() const { return Thing; }
    inline CThing* GetThing() const { return Thing; }
private:
    CThing* Thing;
    CThingPtr* Next;
    CThingPtr* Prev;
};



class CThing : public CReflectionVisitable {
public:
    CThing();
    ~CThing();
    #ifndef WIN32
    static void* operator new(size_t sz);
    #endif
public:
    inline u32 GetUID() const { return UID; }
public:
#ifdef LBP1
    #define PART_MACRO(name, type, cache) inline name* Get##name() const { return (name*)Parts[type]; }
#else
    #define PART_MACRO(name, type, cache) inline name* Get##name() const { return (name*)PartContainer.GetPart(type); }
#endif
    #include "PartList.h"
#undef PART_MACRO
    void AddPart(EPartType type);
    void RemovePart(EPartType type);
public:
    CThingPtr* FirstPtr;
#ifdef LBP1
    union
    {
        CPart* Parts[PART_TYPE_SIZE];
        struct
        {
#define PART_MACRO(name, type, cache) name* Part_##name;
            #include <PartList.h>
#undef PART_MACRO
        } DebugParts;
    };
    PWorld* World;
#else
    PWorld* World;
    CPartContainer PartContainer;
#endif
    CThing* FirstChild;
    CThing* NextSibling;
    CThing* Parent;
    CThing* Root;
    CThing* GroupHead;

#ifndef LBP1
    CThing* MicroChip;
    void* Inputs;
    CThing* BodyRoot;
    void* JointList;
    CGUID PlanGUID;
    u32 UID;
    u16 CreatedBy;
    u16 ChangedBy;
    u16 Flags;
    u16 m0;
    u16 m1;
    u8 ObjectType;
    bool m2;
#else
    CThing* OldEmitter;
    PBody* BodyRoot;
    CRawVector<PJoint*> JointList;
    u32 PlanGUID;
    u32 UID;
    u16 CreatedBy;
    u16 ChangedBy;
    bool Stamping;
#endif

};

#undef PART_MACRO