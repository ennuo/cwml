#pragma once

typedef void (InitStepCallback_Close)(void);
typedef bool (InitStepCallback_Init)(void);

struct SCWLibOptions {
    inline SCWLibOptions()
    {
        InitialiseNetwork = true;
        InitialiseSmallGfxMemPools = false;
        InitialiseForGame = false;
        InstallUnhandledExceptionFilter = true;
        InitialiseGfxMemPools = true;
        InitialiseAllocators = true;
    }

    bool InitialiseForGame;
    bool InstallUnhandledExceptionFilter;
    bool InitialiseGfxMemPools;
    bool InitialiseSmallGfxMemPools;
    bool InitialiseAllocators;
    bool InitialiseNetwork;
};

class CInitStep {
public:
    inline CInitStep() { memset(this, 0, sizeof(CInitStep)); }
    inline CInitStep(const char* name)
    {
        memset(this, 0, sizeof(CInitStep));
        DebugText = name;
    }

    inline CInitStep& SetInitFunc(InitStepCallback_Init* func)
    {
        InitFunc = func;
        return *this;
    }

    inline CInitStep& SetCloseFunc(InitStepCallback_Close* func)
    {
        CloseFunc = func;
        return *this;
    }

    inline CInitStep& SetPostResourceInitFunc(InitStepCallback_Init* func)
    {
        PostResourceInitFunc = func;
        return *this;
    }

public:
    const char* DebugText;
    bool* Check_This_Bool_Before_Init;
    InitStepCallback_Init* InitFunc;
    InitStepCallback_Close* CloseFunc;
    InitStepCallback_Init* PostResourceInitFunc;
    bool Inited;
    CInitStep* ChainTo;
#ifndef LBP1
    void* _Pad0;
    void* _Pad1;
#endif
};

void AddInitSteps(CInitStep* newsteps);
CInitStep* FindInitStep(const char* name);
CInitStep* FindInitStep(CInitStep* step, const char* name);
CInitStep* GetNextInitStep(CInitStep* step);

extern CInitStep gInitSteps[];
extern CInitStep gPs3Test1InitSteps[];

bool InitCWLib(const SCWLibOptions& options, InitStepCallback_Init* start_func);
void CloseCWLib(InitStepCallback_Close* stop_func);

void UpdateWantQuit();
void SetWantQuit(bool wantQuit);
bool WantQuit();
bool WantQuitOrWantQuitRequested();
u64 GetQuitTime();
bool WantInstall();

bool InitJobManager();
void CloseJobManager();