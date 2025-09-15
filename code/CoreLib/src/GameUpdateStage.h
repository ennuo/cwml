#pragma once

enum EMainGameUpdateStage {
    E_UPDATE_STAGE_SYNCED,
    E_UPDATE_STAGE_PREDICTED_OR_RENDER,
    E_UPDATE_STAGE_OTHER_WORLD,
    E_UPDATE_STAGE_LOADING,
    E_UPDATE_STAGE_COUNT
};

extern EMainGameUpdateStage gMainGameUpdateStage;

class CMainGameStageOverride {
public:
    inline CMainGameStageOverride(EMainGameUpdateStage stage) : PrevMainGameUpdateStage(gMainGameUpdateStage)
    {
        gMainGameUpdateStage = stage;
    }
    
    inline ~CMainGameStageOverride()
    {
        gMainGameUpdateStage = PrevMainGameUpdateStage;
    }
private:
    EMainGameUpdateStage PrevMainGameUpdateStage;
};