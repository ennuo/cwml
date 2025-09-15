#pragma once

#include <vector.h>
#include <MMString.h>

#include <Part.h>
#include <thing.h>

class RLevel;

class CEditorSelection : public CReflectionVisitable {
public:
    inline const char* GetName() const { return Name.c_str(); }
    inline void SetName(const char* name) { Name = name; }
    inline const CVector<CThingPtr>& GetThings() const { return Things; }
    inline void SetThings(const CVector<CThingPtr>& things) { Things = things; }
private:
    MMString<char> Name;
    CVector<CThingPtr> Things;
};

class PWorld : public CPart  {
public:
    inline u32 GetNumThings() const { return Things.size(); }
    inline const CRawVector<CThing*>& GetThings() const { return Things; }
    inline CThing* GetThingByIndex(u32 index) const { return Things[index]; }
public:
#define PART_MACRO(name, type, cache) const CRawVector<name*>& GetList##name() const { return List##name; }
    #include "PartList.h"
#undef PART_MACRO
private:
    u32 ThingUIDCounter;
    CRawVector<CThing*> Things; // 0x10 -> 0x1c
    RLevel* Level; // 0x1c
#ifndef LBP1
#ifdef HAS_MOVE_UPDATE
    char mPad000[0x20]; // 0x20
#else
    char mPad000[0x18];
#endif
    u32 GameMode; // 0x40
    float BlueTeamScore; // 0x44
    float RedTeamScore; // 0x48
    bool SubLevel; // 0x4c
#ifdef HAS_MOVE_UPDATE
    bool UseEvenNewerCheckpointCode; // 0x4d
    u8 MinPlayers; // 0x4e
    u8 MaxPlayers;  // 0x4f
    bool MoveRecommended; // 0x50
#endif // HAS_MOVE_UPDATE
#endif // LBP1
    CRawVector<CEditorSelection*> Selections; // 0x54
    CVector<CThingPtr> DissolvingThings; // 0x60
    CVector<CThingPtr> OldDissolvingThings; // 0x6c
#define PART_MACRO(name, type, cache) CRawVector<name*> List##name;
    #include "PartList.h"
#undef PART_MACRO

};