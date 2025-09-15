#ifdef PS3

#include <Ib/Printf.h>
#include <Ib/Emu.h>
#include <Ib/Memory.h>
#include <Ib/Assembly/PowerPC.h>
using namespace Ib;

#include <sys/process.h>
#include <cell/fs/cell_fs_file_api.h>
#include <sys/return_code.h>
#include <string.h>

static bool gIsRPCS3 = false;

static uint32_t gPatchHash;

bool Ib::NeedsRebuildPatch()
{
    if (!IsEmulator()) return false;

    if (IsUsingRecompiler() || gPatchHash != 0)
    {
        if (WriteCache != NULL && WriteCache->GetHash() != gPatchHash)
            return true;
    }

    return false;
}


bool Ib::IsEmulator()
{
    static bool checked = false;
    if (!checked)
    {
        // For now, RPCS3 always uses a PID of 1, while the PS3
        // itself uses some random higher PID, keep it a somewhat high
        // value in case they do add more processes at some point.
        gIsRPCS3 = sys_process_getpid() < 0x1000;
        checked = true;
    }

    return gIsRPCS3;
}

bool __attribute__ ((noinline)) Ib::IsUsingRecompiler()
{
    static bool checked = false;
    if (!checked)
    {
        // Fun little gimmick, overwrite the function we're currently running!!
        uint32_t shellcode[] =
        {
            LI(3, 0),
            BLR
        };

        Ib_Write(*(uint32_t*)&Ib::IsUsingRecompiler, shellcode, sizeof(shellcode));

        checked = true;
        return IsUsingRecompiler();
    }

    return IsEmulator();
}

int Ib::GeneratePatchYML(const char* patch_name, const char* game_name, const char* title_id, const char* output_path)
{
    printf("ib: writing patch yml. patch_name=%s, game_name=%s, title_id=%s, output_path=%s\n", patch_name, game_name, title_id, output_path);

    if (WriteCache == NULL) return CELL_OK;

    uint32_t hash = WriteCache->GetHash();
    Ib_Write(&gPatchHash, &hash, sizeof(uint32_t));

    printf("ib: computed patch hash: %08x\n", gPatchHash);

    int fd, err;
    if ((err = cellFsOpen(output_path, CELL_FS_O_WRONLY | CELL_FS_O_CREAT | CELL_FS_O_TRUNC, &fd, NULL, 0)) != CELL_OK)
        return err;

    char buf[1024];
    char module_name[256];
    #define WRITE_STRING(s) cellFsWrite(fd, (const void*)s, strlen(s), NULL);

    
    WRITE_STRING("Version: 1.2\n\n");

    for (int i = 0; i < WriteCache->GetNumModules(); ++i)
    {
        ModuleSource& source = WriteCache->Sources[i];

        strcpy(buf, source.Filename);
        char* ext = strrchr(buf, '.');
        if (ext != NULL) *ext = '\0';
        char* filename = strrchr(buf, '/');
        filename = filename == NULL ? buf : filename + 1;
        strcpy(module_name, filename);

        int num_writes = WriteCache->GetNumWritesToModule(i); 
        if (num_writes == 0)
        {
            printf("ib: skipping %s because no writes were made\n", module_name);
            continue;
        }

        printf("ib: writing %s with %d writes\n", module_name, num_writes);
        
        bool prx = source.PrxId != 0;


        sprintf(
            buf,
            "%s:\n"
            "  \"%s (%s)\":\n"
            "    Games:\n"
            "      \"%s\":\n"
            "        %s: [ All ]\n"
            "    Author: \"ennuo\"\n"
            "    Patch Version: 1.0\n"
            "    Patch:\n",
            source.GetExecutableHash(),
            patch_name,
            module_name,
            game_name,
            title_id
        );

        WRITE_STRING(buf);
        for (uint32_t i = 0; i < WriteCache->NumWrites; ++i)
        {
            const EmulatorWriteCache::Write& write = WriteCache->Writes[i];
            if (write.Address >= source.MinAddress && write.Address < source.MaxAddress)
            {
                sprintf(buf, "      - [ be32, 0x%x, 0x%08x ]\n", write.Address - source.MinAddress, write.Word);
                WRITE_STRING(buf);
            }
        }

        WRITE_STRING("\n");
    }

    #undef WRITE_STRING
    cellFsClose(fd);
    return CELL_OK;
}

#endif // PS3