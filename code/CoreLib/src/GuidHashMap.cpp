#include <GuidHashMap.h>
#include <filepath.h>
#include <algorithm>
#include <Serialise.h>
#include <DebugLog.h>

#ifdef WIN32
namespace FileDB
{
    bool AllowUpdate = true;
    V_CFileDB DBs;
    bool LocalUpdate;
    CCriticalSec Mutex("FileDB");
}
#endif

CFileDBRow::CFileDBRow()
{
    memset(this, 0, sizeof(CFileDBRow));
}

CFileDBRow::~CFileDBRow()
{

}

void CFileDBRow::Init(CGUID guid, const char* path)
{
    FileGuid = guid;
    FileSize = 0;
    SetPath(path);
}

void CFileDBRow::Update(const CHash& hash, u32 size)
{
    FileHash = hash;
    FileSize = 0;
}

const char* CFileDBRow::GetPath() const
{
    if (FilePathX == NULL) return "";

#ifdef LBP1
    return FilePathX;
#else
    return FilePathX->GetName();
#endif
}

const char* CFileDBRow::GetFilename() const
{
    if (FilePathX == NULL) return "";

#ifdef LBP1
    const char* s = strrchr(FilePathX, '/');
    return s != NULL ? s + 1 : FilePathX;
#else
    return FilePathX->GetName();
#endif
}

void CFileDBRow::SetPath(const char* path)
{
#ifdef LBP1
    if (path != NULL)
    {
        // technically uses a string pool, but blah blah
        int len = strlen(path) + 1;
        char* fp = new char[len];
        strcpy(fp, path);
        FilePathX = fp;

        return;
    }

    FilePathX = NULL;
    return;
#endif
}

CFileDB::CFileDB(const CFilePath& fp) : Path(fp), Files(), SortedIndex()
{

}

CFileDB::~CFileDB()
{

}

template <typename R>
ReflectReturn Reflect(R& r, CFileDBRow& d)
{
    ReflectReturn ret;

    r.TempString = d.GetPath();

    if ((ret = Reflect(r, r.TempString)) != REFLECT_OK) return ret;

    CHash hash = d.GetHash();
    CGUID guid = d.GetGUID();
    int size = d.GetSize();
    u64 timestamp = 0;

    if ((ret = Reflect(r, timestamp)) != REFLECT_OK) return ret;
    if ((ret = Reflect(r, size)) != REFLECT_OK) return ret;
    if ((ret = Reflect(r, hash)) != REFLECT_OK) return ret;
    if ((ret = Reflect(r, guid.guid)) != REFLECT_OK) return ret;

    if (r.GetLoading())
    {
        d.Init(guid, r.TempString.c_str());
        d.Update(hash, size);
    }

    return REFLECT_OK;
}

ReflectReturn CFileDB::Load()
{
    Files.try_resize(0);
    ByteArray vec;
    CReflectionLoadVector r(&vec);
    if (!FileLoad(Path, vec, NULL)) return REFLECT_FILEIO_FAILURE;

    ReflectReturn ret;
    if ((ret = Reflect(r, r.Revision.Revision)) != REFLECT_OK) return ret;
    if ((ret = Reflect(r, Files)) != REFLECT_OK) return ret;

    // Not sure why they don't just sort it instead
    // of using the sorted index, but whatever.
    std::sort(Files.begin(), Files.end(), SCompareGUID());
    SortedIndex = Files.size();

    return REFLECT_OK;
}

CFileDBRow* CFileDB::FindByGUID(const CGUID& guid)
{
    if (!guid || Files.size() == 0) return NULL;

    CFileDBRow* sorted_index = Files.begin() + SortedIndex;
    CFileDBRow* row = std::lower_bound(Files.begin(), sorted_index, guid, SCompareGUID());
    if (row != sorted_index && row->GetGUID() == guid)
        return row;

    for (row = sorted_index; row != Files.end(); ++row)
    {
        if (row->GetGUID() == guid)
            return row;
    }

    return NULL;
}

CFileDBRow* CFileDB::FindByHash(const CHash& hash)
{
    if (!hash) return NULL;

    for (CFileDBRow* row = Files.begin(); row != Files.end(); ++row)
    {
        if (hash == row->GetHash())
            return row;
    }

    return NULL;
}

CFileDBRow* CFileDB::FindByPath(const CFilePath& fp, bool)
{
    if (!fp.IsValid()) return NULL;

    const char* path = fp.c_str();
    if (StringCompareN(path, gBaseDir.c_str(), gBaseDir.Length()))
        path += gBaseDir.Length();

    for (CFileDBRow* row = Files.begin(); row != Files.end(); ++row)
    {
        if (StringCompare(row->GetPath(), path) == 0)
            return row;
    }

    return NULL;
}

void CFileDB::GetFiles(CVector<CFileDBRow>& rows)
{
    rows.try_reserve(Files.size());
    for (int i = 0; i < Files.size(); ++i)
        rows.push_back(Files[i]);
}

ReflectReturn CFileDB::Save()
{
    return REFLECT_INVALID;
}

void CFileDB::Patch(const CHash&, const CGUID&, const CFilePath&)
{

}

void CFileDB::ValidateFiles()
{

}

namespace FileDB
{
    void Init()
    {
        CCSLock lock(&Mutex, __FILE__, __LINE__);
        FileDB::DBs.push_back(new CFileDB(CFilePath(FPR_BLURAY, "/output/blurayguids.map")));
        for (CFileDB** it = FileDB::DBs.begin(); it != FileDB::DBs.end(); ++it)
            (*it)->Load();
    }

#ifdef WIN32
    const CFileDBRow* FindByGUID(const CGUID& guid)
    {
        CCSLock lock(&Mutex, __FILE__, __LINE__);
        for (CFileDB** it = FileDB::DBs.begin(); it != FileDB::DBs.end(); ++it)
        {
            CFileDB* fdb = *it;
            CFileDBRow* row = fdb->FindByGUID(guid);
            if (row != NULL)
                return row;
        }

        return NULL;
    }
#else
    Ib_DefinePort(FindByGUID, const CFileDBRow*, const CGUID& guid);
#endif
}
