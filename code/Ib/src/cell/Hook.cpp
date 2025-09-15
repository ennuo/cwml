#include <Ib/Hook.h>
#include <Ib/Memory.h>
#include <Ib/Assembly/PowerPC.h>
#include <new>

#include <sys/process.h>
#include <sys/syscall.h>
#include <sys/prx.h>

using namespace Ib;

static uint8_t* gBranchPool;
static uint8_t* gBranchPoolBase;
static uint8_t* gBranchPoolFree;

static uint64_t gNextHookIndex = 1;

namespace Ib
{
    void Initialize(InitArgs* args)
    {
        ProcessID = sys_process_getpid();

        if (!args) return;

        gBranchPool = (uint8_t*)args->ExecutableData;
        gBranchPoolBase = gBranchPool;
        gBranchPoolFree = gBranchPool;
        WriteCache = (EmulatorWriteCache*)args->WriteCache;
    }

    void* Allocate(size_t size)
    {
        void* data = (void*)gBranchPoolFree;
        gBranchPoolFree += size;
        return data;
    }

    void* Shellcode(void* data, size_t size)
    {
        void* stub = Allocate(size);
        Ib_Write(stub, data, size);
        return stub;
    }


    HookHandle ReplaceCall(uintptr_t address, const void* hook)
    {
        uint32_t hook_fn_data = ((uint32_t*)hook)[0];
        uint32_t hook_fn_toc = ((uint32_t*)hook)[1];


        const uint32_t FUNCTION_CALL_INDEX = 3;
        const uint32_t BRANCH_INDEX = 5;

        uint32_t shellcode[] =
        {
            // Store the current TOC base
            0xf8410028, // std %r2, 0x28(%r1)

            // Fixup our TOC base and push the function address
            LIS(2, (hook_fn_toc >> 16)),
            ORI(2, 2, (hook_fn_toc & 0xffff)),

            0xDEADBEEF, // Function call address to be replaced

            // Restoure our TOC base and branch back
            0xe8410028, // ld r2, 0x28(r1)
            0xDEADBEEF
        };

        void* stub = Allocate(sizeof(shellcode));
        if (stub == NULL)
            return INVALID_HOOK_HANDLE;
        
        shellcode[FUNCTION_CALL_INDEX] = BL(hook_fn_data, (uint32_t*)stub + FUNCTION_CALL_INDEX);
        shellcode[BRANCH_INDEX] = B((uint32_t)address + 4, (uint32_t*)stub + BRANCH_INDEX);
        
        Ib_Write(stub, shellcode, sizeof(shellcode));

        uint32_t branch = B(stub, address);
        Ib_Write(address, &branch, sizeof(uint32_t));

        return gNextHookIndex++;
    }


    HookHandle Hook(uintptr_t address, const void* func)
    {
        uint32_t hook_fn_data = ((uint32_t*)func)[0];
        uint32_t hook_fn_toc = ((uint32_t*)func)[1];

        // This is excessive on the codesize,
        // but I don't really care too much honestly.
        uint32_t shellcode[] =
        {
            // hack for hooking function imports
            0xf8410028, // std %r2, 0x28(%r1)
            
            // Function prologue
            0xf821ff01, // stdu %r1, -0x100(%r1)
            0x7c0802a6, // mflr %r0
            0xf8010110, // std %r0, 0x110(%r1)
            0xf8410028, // std %r2, 0x28(%r1)

            // Fixup our TOC base and push the function address
            LIS(2, (hook_fn_toc >> 16)),
            ORI(2, 2, (hook_fn_toc & 0xffff)),
            LIS(0, (hook_fn_data >> 16)),
            ORI(0, 0, (hook_fn_data & 0xffff)),

            // Call the hook function
            0x7c0903a6, // mtctr %r0
            0x4e800421, // bctrl

            // Epilogue
            0xe8410028, // ld r2, 0x28(r1)
            0xe8010110, // ld r0, 0x110(r1)
            0x7c0803a6, // mtlr r0
            0x38210100, // addi r1, r1, 0x100
            0x4e800020 // blr
        };

        void* stub_fn = Shellcode(shellcode, sizeof(shellcode));
        if (stub_fn == NULL)
            return INVALID_HOOK_HANDLE;
        
        uint32_t branch = B(stub_fn, address);
        Ib_Write(address, &branch, sizeof(uint32_t));

        return gNextHookIndex++;
    }
}