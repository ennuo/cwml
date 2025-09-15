#include <sys/prx.h>
#include <cwml/Context.h>


SYS_MODULE_INFO(cwml, 0, 1, 0);
SYS_MODULE_START(_start);
SYS_MODULE_STOP(_stop);

typedef void (*func_ptr) (void);
extern func_ptr __CTOR_LIST__[];
extern func_ptr __CTOR_END__[];

static uint8_t gStubData[4096] __attribute__((section(".text#"))) = { 0 };
static Ib::EmulatorWriteCache gWriteCache;

void GetArgumentsForHookAPI(Ib::InitArgs& args)
{
    args.ExecutableData = gStubData;
    if (Ib::IsEmulator())
        args.WriteCache = &gWriteCache;
}

void cwml::GameContext::GetSharedHookArguments(Ib::InitArgs& args) const
{
    GetArgumentsForHookAPI(args);
}

extern void GoLoco();
extern "C" int _start()
{
    __SIZE_TYPE__ nptrs = ((__SIZE_TYPE__)__CTOR_END__ - (__SIZE_TYPE__)__CTOR_LIST__) / sizeof(__SIZE_TYPE__);
    for (unsigned i = 0; i < nptrs; ++i)
        __CTOR_LIST__[i]();

    Ib::InitArgs args;
    memset(&args, 0, sizeof(Ib::InitArgs));
    GetArgumentsForHookAPI(args);
    Ib::Initialize(&args);
    
	GoLoco();
	
    return SYS_PRX_START_OK;
}

extern "C" int _stop()
{
    return SYS_PRX_STOP_OK;
}

extern "C" void __cxa_pure_virtual()
{

}