#pragma once

#include <stdint.h>

#ifdef PS3

#include <sys/prx.h>

#define Ib_Write(address, data, size) Ib::WriteProcessMemory((uintptr_t)(address), (void*)(data), (size), true);
#define Ib_WriteNoCache(address, data, size) Ib::WriteProcessMemory((uintptr_t)(address), (void*)(data), (size), false);
#define Ib_Read(address, data, size) Ib::ReadProcessMemory((uintptr_t)(address), (void*)(data), (size));

#define MAX_WRITES (1024)

namespace Ib
{
    struct ModuleSource
    {
        char Filename[256];
        char ExecutableHash[64];
        sys_prx_id_t PrxId;

        uintptr_t MinAddress;
        uintptr_t MaxAddress;

        const char* GetExecutableHash();
    };

    enum EModuleSource {
        eModuleSource_Executable,
        eModuleSource_FirstRegisteredModule,
        eModuleSource_LastRegisteredModule = 32,
        eModuleSource_Count
    };

    extern sys_pid_t ProcessID;
    
    uint32_t ReadProcessMemory(uintptr_t address, void* data, size_t size);
    uint32_t WriteProcessMemory(uintptr_t address, void* data, size_t size);
    uint32_t WriteProcessMemory(uintptr_t address, void* data, size_t size, bool cache);

    class EmulatorWriteCache {
    public:
        EmulatorWriteCache();
    public:
    struct Write
    {
        inline Write() {}
        inline Write(uintptr_t address, uint32_t word) : Address(address), Word(word) {}

        uint32_t Address;
        uint32_t Word;
    };
    public:
        void Cache(uintptr_t address, uint32_t word);
        void Cache(uintptr_t address, void* data, size_t size);
        uint32_t GetHash() const;
    public:
        void AddExecutableModule(const char* path);
        void AddPRX(sys_prx_id_t id);
        inline uint32_t GetNumModules() const { return NumSources; }
        uint32_t GetNumWritesToModule(uint32_t index) const;
    public:
        ModuleSource Sources[eModuleSource_Count];
        uint32_t NumSources;
        Write Writes[MAX_WRITES];
        uint32_t NumWrites;
    };

    extern EmulatorWriteCache* WriteCache;
}

inline void Ib_Poke32(uintptr_t address, uint32_t value)
{
    Ib_Write((uint64_t)address, (void*)&value, sizeof(uint32_t));
}

#endif
