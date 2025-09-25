#pragma once

#include <GuidHash.h>
#include <CritSec.h>
#include <CalendarTime.h>
#include <vector.h>
#include <filepath.h>
#include <SerialiseEnums.h>
#include <StringUtil.h>

class CFileDBRow {
public:
    CFileDBRow();
    ~CFileDBRow();
public:
    const char* GetPath() const;
    const char* GetFilename() const;
    inline const CHash& GetHash() const { return FileHash; }
    inline const CGUID& GetGUID() const { return FileGuid; }
    inline u32 GetSize() const { return FileSize; }
    void SetPath(const char* path);
    void Update(const CHash& hash, u32 size);
    void Init(CGUID guid, const char* path);
private:
#ifndef LBP1
    class CFileDBPath {
    public:
        inline const CFileDBPath* GetParent() const { return Parent; }
        inline const char* GetName() const { return Name; }
    private:
        const CFileDBPath* Parent;
        char Name[255];
    };

    CHash FileHash;
    CGUID FileGuid;
    u32 FileSize;
    const CFileDBPath* FilePathX;
#else
    const char* FilePathX;
    CHash FileHash;
    CGUID FileGuid;
    u32 FileSize;
#endif
};

typedef CVector<CFileDBRow> V_CFileDBRow;

struct SComparepath
{
    inline bool operator()(const CFileDBRow& lhs, const CFileDBRow& rhs)
    {
        return StringCompare(lhs.GetPath(), rhs.GetPath());
    }

    inline bool operator()(const CFileDBRow& lhs, const CFilePath& rhs)
    {
        return StringCompare(lhs.GetPath(), rhs.c_str());
    }

    inline bool operator()(const CFilePath& lhs, const CFileDBRow& rhs)
    {
        return StringCompare(lhs.c_str(), rhs.GetPath());
    }
};

struct SCompareGUID
{
    inline bool operator()(const CFileDBRow& lhs, const CFileDBRow& rhs)
    {
        return lhs.GetGUID() < rhs.GetGUID();
    }

    inline bool operator()(const CGUID& lhs, const CFileDBRow& rhs)
    {
        return lhs < rhs.GetGUID();
    }

    inline bool operator()(const CFileDBRow& lhs, const CGUID& rhs)
    {
        return lhs.GetGUID() < rhs;
    }
};


class CFileDB {
public:
    CFileDB(const CFilePath& fp);
    virtual ~CFileDB();
    ReflectReturn Load();
    virtual CFileDBRow* FindByGUID(const CGUID& guid);
    virtual CFileDBRow* FindByHash(const CHash& hash);
    virtual CFileDBRow* FindByPath(const CFilePath& fp, bool);
    void GetFiles(CVector<CFileDBRow>& rows);
    virtual ReflectReturn Save();
    virtual void Patch(const CHash&, const CGUID&, const CFilePath&);
    virtual void ValidateFiles();
protected:
    CFilePath Path;
    V_CFileDBRow Files;
    u32 SortedIndex;
};

typedef CVector<CFileDB*> V_CFileDB;

namespace FileDB
{
    extern bool AllowUpdate;
    extern V_CFileDB DBs;
    extern bool LocalUpdate;
    extern CCriticalSec Mutex;

    void Init();

#ifndef WIN32
    Ib_DeclarePort(FindByGUID, const CFileDBRow*, const CGUID& guid);
    Ib_DeclarePort(FindByHash, const CFileDBRow*, const CHash& guid);
#else
    const CFileDBRow* FindByGUID(const CGUID& guid);
    const CFileDBRow* FindByHash(const CHash& hash);
#endif
}