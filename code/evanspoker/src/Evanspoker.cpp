#include <sys/prx.h>
#include <sys/ppu_thread.h>
#include <cell/fs/cell_fs_file_api.h>
#include <cell/sysmodule.h>
#include <sys/return_code.h>

#include <filepath.h>
#include <GuidHash.h>
#include <algorithm>
#include <set>

#include <Ib/Printf.h>

#include <sha1.h>

#include <DebugLog.h>

#include <FartRO.h>

class CFileDBRow {
public:
	inline bool operator==(CFileDBRow const& rhs) const { return FileGuid == rhs.FileGuid; }
	inline bool operator!=(CFileDBRow const& rhs) const { return FileGuid != rhs.FileGuid; }
	inline bool operator<(CFileDBRow const& rhs) const { return FileGuid < rhs.FileGuid; }
	inline bool operator<=(CFileDBRow const& rhs) const { return FileGuid <= rhs.FileGuid; }
	inline bool operator>(CFileDBRow const& rhs) const { return FileGuid > rhs.FileGuid; }
	inline bool operator>=(CFileDBRow const& rhs) const { return FileGuid >= rhs.FileGuid; }
public:
    CHash FileHash;
    const char* FilePathX;
    CGUID FileGuid;
    CHash* OriginalHash;
    u32 Flags;
};

struct FileDBHeader
{
    u32 Revision;
    u32 NumFiles;
};

char LocalFilePathBuffer[1000 * 1000];
char* NextFreePath = LocalFilePathBuffer;
CFileDBRow LocalRows[16384];
u32 NumRows;

CFileDBRow* HackFixupZeroedEntries()
{
    for (int i = 0; i < NumRows; ++i)
    {
        CFileDBRow& row = LocalRows[i];
        if (row.OriginalHash != NULL)
            *row.OriginalHash = row.FileHash;
        // row.OriginalHash = NULL;
        // row.Flags &= ~2;
    }

    return LocalRows;
}

void GoingEvans()
{
    if (!cellSysmoduleIsLoaded(CELL_SYSMODULE_FS))
        cellSysmoduleLoadModule(CELL_SYSMODULE_FS);

    char fp[MAX_PATH];

    FileHandle fd;
    FileOpen("/dev_hdd0/game/BCET70002/USRDIR/output/blurayguids.map", fd, OPEN_READ);

    FileDBHeader header;
    FileRead(fd, &header, sizeof(FileDBHeader));
    for (int i = 0; i < header.NumFiles; ++i)
    {
        u32 len;
        FileRead(fd, &len, sizeof(u32));

        CFileDBRow& row = LocalRows[NumRows++];
        row.OriginalHash = NULL;

        row.FilePathX = NextFreePath;
        FileRead(fd, NextFreePath, len);
        NextFreePath += (len + 1);

        FileSeek(fd, 0xc, FILE_CURRENT);

        FileRead(fd, &row.FileHash, sizeof(CHash));
        FileRead(fd, &row.FileGuid, sizeof(CGUID));

        if (row.FileHash == CHash::Zero)
        {
            sprintf(fp, "/dev_hdd0/game/BCET70002/USRDIR/%s", row.FilePathX);

            MMLog("recomputing sha1 for %s\n", fp);
            FileHandle rfd;
            if (FileOpen(fp, rfd, OPEN_READ))
            {
                char buf[1024];
                CSHA1Context ctx;

                while (true)
                {
                    int n = FileRead(rfd, buf, 1024);
                    if (n <= 0) break;
                    ctx.AddData((uint8_t*)buf, n);
                }

                ctx.Result((u8*)&row.FileHash);
                FileClose(rfd);

                char hs[CHash::kHashHexStringSize];
                row.FileHash.ConvertToHex(hs);
                MMLog("\t-> h%s\n", hs);
            }
        }

        row.Flags = 2;
    }

    FileClose(fd);

    MMLog("loaded %d entries from file database\n", NumRows);

    std::sort(LocalRows, LocalRows + NumRows, std::less<CFileDBRow>());
}

void GoLoco()
{
    MMLog("WE GOING EVANS!\n");
    MMLog("            __\n");
    MMLog("(\\,--------'()'--o\n");
    MMLog(" (_    ___    /~\"\n");
    MMLog("  (_)_)  (_)_)\n");

    Ib_Poke32(0x009b4eb8, (u32)&NumRows);
    Ib_Poke32(0x009b4eb4, (u32)&LocalRows);
    Ib_PokeCall(0x0043b31c, HackFixupZeroedEntries);
    Ib_PokeCall(0x0043b290, GoingEvans);

    for (int i = 0; i < Ib::WriteCache->NumWrites; ++i)
    {
        Ib::EmulatorWriteCache::Write& write = Ib::WriteCache->Writes[i];
        MMLog("      - [ be32, 0x%08x, 0x%08x ]\n", write.Address, write.Word);
    }
}