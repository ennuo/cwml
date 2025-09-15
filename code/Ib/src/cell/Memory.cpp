#ifdef PS3

#include <Ib/Memory.h>
#include <Ib/Emu.h>
#include <Ib/Printf.h>
#include <Ib/SHA1.h>
using namespace Ib;

#include <stddef.h>

#include <sys/process.h>
#include <sys/syscall.h>
#include <sys/prx.h>

#include <cell/fs/cell_fs_file_api.h>
#include <sys/return_code.h>
#include <string.h>
#include <np/drm_inline.h>


#define MIN_MACRO(a,b) ((a)<(b)?(a):(b))
#define MAX_MACRO(a,b) ((a)>(b)?(a):(b))

sys_pid_t Ib::ProcessID = sys_process_getpid();
#define INVALID_SEGMENT_BASE (~0ul)
static uintptr_t gSegmentBase = INVALID_SEGMENT_BASE;

EmulatorWriteCache* Ib::WriteCache;

Ib::EmulatorWriteCache::EmulatorWriteCache()
{
    memset(this, 0, sizeof(Ib::EmulatorWriteCache));
}

void Ib::EmulatorWriteCache::Cache(uintptr_t address, uint32_t word)
{
    Writes[NumWrites++] = Write(address, word);
}

void Ib::EmulatorWriteCache::Cache(uintptr_t address, void* data, size_t size)
{
    int num_words = size / sizeof(uint32_t);
    for (int i = 0; i < num_words; ++i)
        Writes[NumWrites++] = Write(address + (i * sizeof(uint32_t)), *(((uint32_t*)data) + i));
}

uint32_t Ib::EmulatorWriteCache::GetHash() const
{
    const char* begin = (const char*)Writes;
    const char* end = (const char*)(Writes + NumWrites);

    const uint32_t prime = 0x1000193ul;
    uint32_t hash = 0x811c9dc5;

    while (begin != end)
    {
        hash *= prime;
        hash ^= *begin++;
    }

    return hash;
}

void Ib::EmulatorWriteCache::AddExecutableModule(const char* path)
{
    printf("ib: caching executable path=%s\n", path);

    ModuleSource& source = Sources[NumSources++];
    strcpy(source.Filename, path);
    strcpy(source.ExecutableHash, "PPU-b7d59e2bd332611fa249066dc6646983aad0a02d");
    source.MaxAddress = ~0ul;
}

void Ib::EmulatorWriteCache::AddPRX(sys_prx_id_t id)
{
    char path[256];
    sys_prx_segment_info_t segments[32];
    sys_prx_module_info_t info;
    memset(&info, 0, sizeof(sys_prx_module_info_t));
    info.size = sizeof(sys_prx_module_info_t);
    info.segments_num = 32;
    info.segments = segments;
    info.filename = path;
    info.filename_size = 256;

    if (sys_prx_get_module_info(id, 0, &info) != CELL_OK)
    {
        printf("ib: failed to get info for prx %d\n", id);
        return;
    }

    printf("ib: caching prx path=%s, id=%d\n", path, (uint32_t)id);

    ModuleSource& source = Sources[NumSources++];
    strcpy(source.Filename, path);
    strcpy(source.ExecutableHash, "PRX-aaaaaaaaaaaaaaaaaaaaaaaaaaaa-0");
    source.PrxId = id;



    uint64_t min = ~0ull;
    uint64_t max = 0;

    if (info.segments_num > 0)
    {
        printf("ib: start addr: %08x\n", info.start_entry);

        for (int i = 0; i < info.segments_num; ++i)
        {
            sys_prx_segment_info_t& seg = segments[i];

            min = MIN_MACRO(min, seg.base);
            max = MAX_MACRO(max, seg.base + seg.memsz);
        }
    }

    source.MinAddress = min;
    source.MaxAddress = max;

    printf("ib: min=%08x max=%08x\n", source.MinAddress, source.MaxAddress);

    ModuleSource& exec = Sources[eModuleSource_Executable];
    if (exec.MaxAddress == ~0ul)
    {
        printf("ib: updating mainexec max addr to %08x\n", source.MinAddress);
        exec.MaxAddress = source.MinAddress;
    }
}

uint32_t Ib::EmulatorWriteCache::GetNumWritesToModule(uint32_t index) const
{
    if (index >= NumSources) return 0;

    uint32_t writes = 0;
    const ModuleSource& source = Sources[index];
    for (uint32_t i = 0; i < NumWrites; ++i)
    {
        const Write& write = Writes[i];
        if (write.Address >= source.MinAddress && write.Address < source.MaxAddress)
            writes++;
    }
    
    return writes;
}

struct ElfHeader
{
    char Magic[4];
    uint8_t Class;
    uint8_t Data;
    uint8_t Ver;
    uint8_t Abi;
    uint8_t AbiVersion;
    char pad[7];
    uint16_t Type;
    uint16_t Machine;
    uint32_t Version;
    uint64_t Entry;
    uint64_t ProgramHeaderOffset;
    uint64_t SectionHeaderOffset;
    uint32_t Flags;
    uint16_t Size;
    uint16_t ProgramHeaderEntrySize;
    uint16_t ProgramHeaderCount;
    uint16_t SectionHeaderEntrySize;
    uint16_t SectionHeaderCount;
    uint16_t SectionHeaderStringIndex;
};

struct CertifiedFile
{
    uint32_t Magic;
    uint32_t Version;
    uint16_t Attribute;
    uint16_t Category;
    uint32_t ExtendedHeaderSize;
    uint64_t FileOffset;
    uint64_t FileSize;
    uint64_t Padding;
};

struct ElfProgramHeader
{
    uint32_t Type;
    uint32_t Flags;
    uint64_t Offset;
    uint64_t VirtualAddress;
    uint64_t PhysicalAddress;
    uint64_t FileSize;
    uint64_t MemorySize;
    uint64_t Alignment;
};

const char* Ib::ModuleSource::GetExecutableHash()
{
    const uint32_t kSCE = 0x53434500ul;
    const uint32_t kELF = 0x7F454C46ul;

    printf("calculating ppu hash for %s\n", Filename);

    int fd;
    if (sceNpDrmOpen(NULL, Filename, 0, &fd, NULL, 0) != CELL_OK)
    {
        printf("ib: failed to open %s to calculate executable hash\n", Filename);
        return ExecutableHash;
    }

    uint64_t dummy64, n;
    uint32_t magic;
    cellFsRead(fd, &magic, sizeof(uint32_t), &n);

    printf("magic: %08x\n", magic);

    uint64_t startpos = 0;
    if (magic == kSCE)
    {
        CertifiedFile sce;
        cellFsLseek(fd, 0, CELL_FS_SEEK_SET, &dummy64);
        cellFsRead(fd, &sce, sizeof(CertifiedFile), &n);
        startpos = sce.FileOffset;
    }

    cellFsLseek(fd, startpos, CELL_FS_SEEK_SET, &dummy64);
    ElfHeader header;
    cellFsRead(fd, &header, sizeof(ElfHeader), &n);

    HashContext ctx;
    ctx.Reset();

    for (int i = 0; i < header.ProgramHeaderCount; ++i)
    {
        uint32_t offset = (startpos + header.ProgramHeaderOffset) + (i * header.ProgramHeaderEntrySize);
        cellFsLseek(fd, offset, CELL_FS_SEEK_SET, &dummy64);

        ElfProgramHeader header;
        cellFsRead(fd, &header, sizeof(ElfProgramHeader), &n);


        ctx.AddData((const uint8_t*)&header.Type, sizeof(uint32_t));
        ctx.AddData((const uint8_t*)&header.Flags, sizeof(uint32_t));


        if (header.Type == 1 && header.MemorySize != 0)
        {
            ctx.AddData((const uint8_t*)&header.VirtualAddress, sizeof(uint64_t));
            ctx.AddData((const uint8_t*)&header.MemorySize, sizeof(uint64_t));



            cellFsLseek(fd, startpos + header.Offset, CELL_FS_SEEK_SET, &dummy64);

            uint64_t size = header.FileSize;
            char buffer[8192];
            while (size > 0)
            {
                cellFsRead(fd, buffer, MIN_MACRO(size, 8192), &n);
                ctx.AddData((const uint8_t*)buffer, n);
                size -= n;
            }
        }
    }

    cellFsClose(fd);

    bool prx = PrxId != 0;
    uint8_t hash[0x14];
    ctx.Result(hash);

    if (prx)
    {
        static const char* Base57Chars = "0123456789ACEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
        char stringify[64];

        const uint8_t tail = 6;
        const uint32_t out_size = sizeof(hash) / 8 * 11 + tail;

        for (uint32_t i = 0, p = 0; i < sizeof(hash); i += 8, p += 11)
        {
            uint64_t value = 0;

            if (sizeof(hash) - i < sizeof(uint64_t))
                memcpy(&value, hash + i, sizeof(hash) - i);
            else
                memcpy(&value, hash + i, sizeof(uint64_t));

            for (int j = 10; j >= 0; --j)
            {
                if (p + j < out_size)
                    stringify[p + j] = Base57Chars[value % 57];
                value /= 57;
            }
        }

        stringify[out_size] = '\0';
        sprintf(ExecutableHash, "PRX-%s-0", stringify);
    }
    else
    {
        static const char* HexChars = "0123456789abcdef";
        char stringify[41];
        for (int i = 0; i < 0x14; ++i)
        {
            uint8_t b = hash[i];
            stringify[i * 2] = HexChars[b >> 4];
            stringify[(i * 2) + 1] = HexChars[b & 0xf];
        }

        stringify[40] = '\0';
        sprintf(ExecutableHash, "PPU-%s", stringify);
    }

    return ExecutableHash;
}

uint32_t Ib::ReadProcessMemory(uintptr_t address, void* data, size_t size)
{
    system_call_4(904, (uint64_t)ProcessID, (uint64_t)address, size, (uint64_t)data);
    return_to_user_prog(uint32_t);
}

uint32_t Ib::WriteProcessMemory(uintptr_t address, void* data, size_t size, bool cache)
{
    if (cache && IsEmulator() && WriteCache != NULL)
        WriteCache->Cache(address, data, size);
    
    system_call_4(905, (uint64_t)ProcessID, (uint64_t)address, size, (uint64_t)data);
    return_to_user_prog(uint32_t);
}

uint32_t Ib::WriteProcessMemory(uintptr_t address, void* data, size_t size)
{
    return WriteProcessMemory(address, data, size, true);
}

#endif // PS3