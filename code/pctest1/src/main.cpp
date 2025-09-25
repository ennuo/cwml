#include <GuidHash.h>
#include <DebugLog.h>
#include <Fart.h>
#include <FartRO.h>

#include <filepath.h>
#include <Directory.h>
#include <thread.h>

#include <ResourceDescriptor.h>
#include <ResourceSystem.h>
#include <Resource.h>
#include <ResourceGuidSubst.h>
#include <ResourceDLC.h>
#include <GuidHashMap.h>
#include <ResourceScript.h>
#include <System.h>
#include <Clock.h>
#include <JobManager.h>
#include <ReadINI.h>

#include <vm/VirtualMachine.h>
#include <vm/NativeFunctionCracker.h>
#include <vm/NativeRegistry.h>

const char* FAKE_ROOT = "E:/emu/rpcs3/dev_hdd0/game/NPUA80472/USRDIR";
const char* FAKE_SYSCACHE = "E:/emu/rpcs3/dev_hdd1/caches/NPUA80472_NPUA80472";

bool BootCheck()
{
    gGameDataPath = FAKE_ROOT;
    gBaseDir = FAKE_ROOT;
    gSysCachePath = FAKE_SYSCACHE;

    return true;
}


CInitStep gInitSteps[] =
{
    CInitStep("PerformanceTimers").SetInitFunc(InitPerformanceTimers),
    CInitStep("Threads").SetInitFunc(InitThreads),
    CInitStep("JobManager")
        .SetInitFunc(InitJobManager)
        .SetCloseFunc(CloseJobManager),
    CInitStep("BootCheck").SetInitFunc(BootCheck),
    CInitStep("Caches")
        .SetInitFunc(InitCaches)
        .SetCloseFunc(CloseCaches),
    CInitStep("ResourceSystem")
        .SetInitFunc(InitResourceSystem)
        .SetCloseFunc(CloseResourceSystem),
    CInitStep("VirtualMachine")
        .SetInitFunc(NVirtualMachine::Initialise)
        .SetCloseFunc(NVirtualMachine::Finalise),
    CInitStep()
};

CInitStep gPcTest1InitSteps[] =
{
    CInitStep()
};



bool SystemInit()
{
    AddInitSteps(gPcTest1InitSteps);
    InitThreads();
    AmInMainThread();

    SCWLibOptions options;
    memset(&options, 0, sizeof(SCWLibOptions));

    return InitCWLib(options, NULL);
}

void SystemClose()
{
    SetWantQuit(true);
    // DrainResourceSystem();
    CloseCWLib(NULL);
}

namespace TestNativeFunctions
{
    void NativeTest()
    {
        MMLog("called from scripting environment!\n");
    }

    void NativeTestInt(int value)
    {
        MMLog("called from scripting environment: %d\n", value);
    }

    int GetIntegerValue()
    {
        return 1337;
    }

    void Register()
    {
        REGISTER_NATIVE_FUNCTION_STATIC("cwml", NativeTest);
        REGISTER_NATIVE_FUNCTION_STATIC("cwml", NativeTestInt);
        REGISTER_NATIVE_FUNCTION_STATIC("cwml", GetIntegerValue);
    }
}

void JobManagerTest(void* userdata)
{
    MMLog("called from job manager! %d\n", RTYPE_LAST);
}

const int E_KEY_OBJECT_FF = 12056;
const int E_KEY_THING_FF = 9355;
const int E_KEY_TESTCLASSDERIVED_FF = 18504;
const int E_KEY_CWML_FF = 2214053154ul;

int main(int argc, char** argv)
{
    printf("pctest1 build date: " __DATE__ " time: " __TIME__ "\n");
    printf("argc %d argv[0]=%s\n", argc, argv[0]);
    if (0 < argc)
        printf("argv[1]=%s\n", argv[1]);

    // DebugLogEnable(DC_INIT, false);
    // DebugLogEnable(DC_RESOURCE, false);

    if (SystemInit() && !WantQuitOrWantQuitRequested())
    {
        TestNativeFunctions::Register();

        CP<RScript> script = RScript::BlockUntilLoaded(E_KEY_TEST_FF);
        if (script)
        {
            MMLogCh(DC_INIT, "\n============== START SCRIPT TEST ==============\n\n");

            NVirtualMachine::CScriptObjectInstance* instance = NVirtualMachine::CScriptObjectInstance::Create(script, NULL, true);
            
            CScriptArguments arguments;
            // arguments.AppendArg(8);

            instance->InvokeSync(NULL, CSignature("main__"), arguments);

            MMLogCh(DC_INIT, "\n============== STOP SCRIPT TEST! ==============\n\n");

        }
    }

    SystemClose();

    float shutdown_time = ToSeconds(GetClock() - GetQuitTime());
    if (shutdown_time <= 10.0f)
        MMLogCh(DC_INIT, "SHUTDOWN in %.3fs - We Rock!\n", shutdown_time);
    else
        MMLogCh(DC_INIT, "TRC FAIL - SHUTDOWN in %.3fs\n", shutdown_time);

    MMLogCh(DC_INIT, "SHUTDOWN... @ %f\n", ToSeconds(GetClock()));

    puts("Leaving main");

    return 0;
}