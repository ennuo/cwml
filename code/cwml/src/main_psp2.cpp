#ifdef VITA

#include <Ib/Hook.h>

extern void GoLoco();

extern "C" void _start() __attribute__((weak, alias("module_start")))
extern "C" int module_start(SceSize argc, const void* args)
{
    Ib::Initialize();
    GoLoco();
    return SCE_KERNEL_START_SUCCESS;
}

extern "C" int module_stop(SceSize argc, const void* args)
{
    Ib::Close();
    return SCE_KERNEL_STOP_SUCCESS;
}

#endif // VITA
