#include <filepath.h>
#include <vector.h>
#include <refcount.h>
#include <GuidHash.h>
#include <GuidHashMap.h>
#include <DebugLog.h>
#include <FartRO.h>
#include <ResourceDescriptor.h>

#include <Ib/Assembly/PowerPC.h>
#include <algorithm>


#define LOG_CHANNEL "evanspoker: "
class CEmbeddedFileDBRow {
public:
    CEmbeddedFileDBRow()
    {
        memset(this, 0, sizeof(CEmbeddedFileDBRow));
    }
public:
    CHash FileHash;
    const char* FilePathX;
    CGUID FileGuid;
    CHash* LatestHash;
    u32 Flags;
};

class CCrapSerialisedResource : public CBaseCounted {
public:
    const CResourceDescriptorBase& GetDescriptor() const { return Descriptor; }
private:
    CResourceDescriptorBase Descriptor;
public:
    ByteArray Data;
    /// ... so on, but i don't really care
};

struct SCompareRow
{

    inline bool operator()(const CEmbeddedFileDBRow& lhs, const CEmbeddedFileDBRow& rhs) const
    {
        return lhs.FileGuid < rhs.FileGuid;
    }

    inline bool operator()(const CEmbeddedFileDBRow& lhs, const CGUID& rhs) const
    {
        return lhs.FileGuid < rhs;
    }

    inline bool operator()(const CGUID& lhs, const CEmbeddedFileDBRow& rhs) const
    {
        return lhs < rhs.FileGuid;
    }
};

CVector<CEmbeddedFileDBRow> gEmbeddedFileDB;

class CCrapFileDB : public CFileDB {
public:
    CCrapFileDB(const CFilePath& path) : CFileDB(path) {}
public:
    void GetEmbeddedFiles(CVector<CEmbeddedFileDBRow>& rows)
    {
        rows.resize(Files.size());
        for (u32 i = 0; i < Files.size(); ++i)
        {
            const CFileDBRow& it = Files[i];
            CEmbeddedFileDBRow& row = rows[i];

            row.FilePathX = it.GetPath();
            row.FileHash = it.GetHash();
            row.FileGuid = it.GetGUID();
        }
    }
};

const bool gUseCaches = true;
CCache* gCaches[CT_COUNT];

void LoadCaches()
{
    CFilePath fart(FPR_GAMEDATA, "data.farc");
    if (FileExists(fart))
    {
        MMLog(LOG_CHANNEL "adding %s to gCaches[CT_READONLY]\n", fart.c_str());
        gCaches[CT_READONLY] = MakeROCache(fart, true, false, false);
    }
    
    for (int i = 0; i < CT_SAVEGAME_LAST - CT_SAVEGAME_FIRST; ++i)
    {
        char filename[255];
        sprintf(filename, "patch%d.farc", i);
        CFilePath patch_fart(FPR_GAMEDATA, filename);
        if (FileExists(patch_fart))
        {
            MMLog(LOG_CHANNEL "adding %s to gCaches[CT_SAVEGAME_FIRST + %d]\n", patch_fart.c_str(), i);
            gCaches[CT_SAVEGAME_FIRST + i] = MakeROCache(patch_fart, true, false, false);
        }
    }
}

bool GetResourceFromCache(CP<CCrapSerialisedResource>& csr)
{
    CHash hash = csr->GetDescriptor().GetHash();
    CGUID guid = csr->GetDescriptor().GetGUID();
    EResourceType type = csr->GetDescriptor().GetType();

    // These should always be loaded from either the PSARC
    // or from the filesystem.
    if (type == RTYPE_FILENAME || type == RTYPE_FONTFACE) 
        return false;

    CEmbeddedFileDBRow* row = NULL;

    // Prefer the latest hash avaiable from the file database.
    if (guid)
    {
        CEmbeddedFileDBRow* it = std::lower_bound(gEmbeddedFileDB.begin(), gEmbeddedFileDB.end(), guid, SCompareRow());
        if (it != gEmbeddedFileDB.end() && it->FileGuid == guid)
        {
            row = it;
            hash = it->FileHash;
        }
    }

    if (row != NULL)
        MMLog(LOG_CHANNEL "loading %s, g%d (%s)\n", StringifyHash(hash).c_str(), guid.guid, row->FilePathX);
    else
        MMLog(LOG_CHANNEL "loading %s, g%d\n", StringifyHash(hash).c_str(), guid.guid);
        
    if (!hash)
    {
        // If we have a zero hash, try checking if the loose
        // file exists and load it.
        if (row != NULL)
        {
            CFilePath fp(FPR_GAMEDATA, row->FilePathX);
            if (!FileExists(fp)) return false;
            return FileLoad(fp, csr->Data, NULL);
        }

        return false;
    }

    // If caches are enabled, try loading the hash from the farcs
    // over the psarc.
    if (gUseCaches)
    {
        SResourceReader reader;
        if (GetResourceReader(hash, reader))
            return FileLoad(reader, csr->Data);
    }

    return false;
}

void InitialiseFileDatabase()
{
    if (gUseCaches) LoadCaches();
    
    MMLog(LOG_CHANNEL "initialising file database\n");
    CFilePath fp(FPR_GAMEDATA, "output/blurayguids.map");
    CCrapFileDB database(fp);
    if (database.Load() != REFLECT_OK)
    {
        MMLog(LOG_CHANNEL "failed to parse %s!\n", fp.c_str());
        return;
    }

    database.GetEmbeddedFiles(gEmbeddedFileDB);

    // Poke the pointers in the TOC to the internal database start/size to our own
    // database vector
    Ib_Poke32(0x009b4eb4, (u32)gEmbeddedFileDB.begin());
    Ib_Poke32(0x009b4eb8, (u32)&gEmbeddedFileDB.GetSizeForSerialisation());

    MMLog(LOG_CHANNEL "loaded %d entries from file database\n", gEmbeddedFileDB.size());
}

int IsSocketConnected()
{
    return CELL_OK;
}

extern "C" void LoopbackPatchHook();
extern "C" void CSRPatchHook();
void GoLoco()
{
    MMLog("WE GOING EVANS!\n");
    MMLog("            __\n");
    MMLog("(\\,--------'()'--o\n");
    MMLog(" (_    ___    /~\"\n");
    MMLog("  (_)_)  (_)_)\n");

    gGameDataPath = "/dev_hdd0/game/BCET70002/USRDIR";

    // Wait until resource system starts to actually load the database,
    // has the added benefit of being after the memory manager initializes.
    Ib_PokeCall(0x0043b290, InitialiseFileDatabase);
    Ib_PokeBranch(0x0008e164, &CSRPatchHook);

    // If we're on emulator, just print out our write cache
    // for LLVM, maybe switch to Ib's built in exporter?
    if (Ib::IsEmulator())
    {
        // sys_net_get_sockinfo isn't implemented on RPCS3,
        // so this will cause the game to basically cripple itself,
        // since everything works over sockets.
        Ib_PokeHook(0x000f33d8, IsSocketConnected);
        Ib_PokeBranch(0x000f4878, &LoopbackPatchHook);

        if (Ib::WriteCache != NULL)
        {
            const char* kAutoPatchRoot = "/dev_usb007";
            const char* kTitleID = "BCET70002";

            Ib::WriteCache->AddExecutableModule(CFilePath(FPR_GAMEDATA, "EBOOT.BIN"));
            Ib::WriteCache->AddPRX(sys_prx_get_my_module_id());

            if (Ib::NeedsRebuildPatch())
            {
                char fp[255];
                if (FileExists(kAutoPatchRoot))
                    sprintf(fp, "%s/%s_patch.yml", kAutoPatchRoot, kTitleID);
                else
                    sprintf(fp, "%s/output/%s_patch.yml", gGameDataPath.c_str(), kTitleID);
                
                Ib::GeneratePatchYML("evanspoker", "LittleBigPlanet Internal Beta", kTitleID, fp);
            }
        }
    }
}