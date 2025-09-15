#pragma once

#include <Resource.h>
#include <GuidHash.h>
#include <MMString.h>
#include <filepath.h>
#include <vector.h>
#include <refcount.h>

struct SDLCFile {
    SDLCFile() : Directory(), File(), ContentID(), InGameCommerceID(), CategoryID(), GUIDs(), Flags()
    {

    }

    MMString<char> Directory;
    MMString<char> File;
    MMString<char> ContentID;
    MMString<char> InGameCommerceID;
    MMString<char> CategoryID;
    CRawVector<u32> GUIDs;
    u32 Flags;
};

struct SDLCGUID {
    SDLCGUID() : GUID(), Flags() {}

    CGUID GUID;
    u32 Flags;
};

class RDLC : public CResource {
public:
    template<typename R>
    friend ReflectReturn Reflect(R&, RDLC&);
public:
    RDLC(EResourceFlag flags);
public:
    static bool SGUIDCompare(const SDLCGUID& lhs, const CGUID& rhs);
    SDLCGUID* Find(const CGUID& guid);
    void AddAllDLC();
    void Merge(const CP<RDLC>&);
    void DLCCheck(const char*);
    void SetDLCFileOwned(SDLCFile& file);
    bool ReadDLC(FileHandle, SDLCFile&);
    bool DoIOwnAnyDLC() const;
    const SDLCFile* FindDLCFile(const char*) const;
    void GetDLCFileListFromGUIDList(const CVector<CGUID>&, const CVector<SDLCFile>&) const;
    inline const CVector<SDLCGUID>& GetGUIDS() const { return GUIDS; }
    inline const CVector<SDLCFile>& GetFiles() const { return Files; }
private:
    CVector<SDLCGUID> GUIDS;
    CVector<SDLCFile> Files;
};
