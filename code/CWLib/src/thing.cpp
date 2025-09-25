#include "thing.h"

CThingPtr::~CThingPtr()
{
    Unset();
}

void CThingPtr::Unset()
{
    if (Thing != NULL)
    {
        if (Next != NULL) Next->Prev = Prev;
        if (Prev == NULL) Thing->FirstPtr = Next;
        else Prev->Next = Next;
    }

    Thing = NULL;
    Next = NULL;
    Prev = NULL;
}

void CThingPtr::Set(CThing* thing)
{
    Thing = thing;
    if (Thing != NULL)
    {
        Next = Thing->FirstPtr;
        Thing->FirstPtr = this;
        if (Next != NULL) Next->Prev = this;
        Prev = NULL;
    }
    else
    {
        Next = NULL;
        Prev = NULL;
    }
}

CThingPtr::CThingPtr() : Thing(NULL), Next(NULL), Prev(NULL)
{

}

CThingPtr::CThingPtr(CThing* thing) : Thing(NULL), Next(NULL), Prev(NULL)
{
    Set(thing);
}

CThingPtr& CThingPtr::operator=(CThingPtr const& rhs) 
{ 
    Unset();
    Set(rhs.Thing);
    return *this;
}

CThingPtr& CThingPtr::operator=(CThing* rhs) 
{ 
    Unset();
    Set(rhs);
    return *this;
}


Ib_DefinePort(CThing_AddPart, void, CThing* thing, EPartType part);
Ib_DefinePort(CThing_RemovePart, void, CThing* thing, EPartType part);
Ib_DefinePort(CThing_Deconstructor, void, CThing* thing);

CThing::CThing()
{
    memset(this, 0, sizeof(CThing));
    Root = this;
    CreatedBy = -1;
    ChangedBy = -1;
}

CThing::~CThing()
{
    CThing_Deconstructor(this);
}

#ifndef WIN32
void* CThing::operator new(size_t sz)
{
    return ::operator new(sz, 256);
}
#endif

void CThing::AddPart(EPartType type) { CThing_AddPart(this, type); }
void CThing::RemovePart(EPartType type) { CThing_RemovePart(this, type); }
