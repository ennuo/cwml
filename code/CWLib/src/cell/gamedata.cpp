#include <filepath.h>

#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_gamecontent.h>
#include <sysutil/sysutil_syscache.h>

#include <DebugLog.h>
#include <StringUtil.h>

u32 gBootType;
u32 gBootAttributes;
s32 gGameDataResult;
CellGameContentSize gGameDataSize;
s64 gTotalSizeNeededKb;
char* gTitleID = "NPUA80662";

bool WantInstall()
{
    return false;
}

// This isn't the actual boot check function, it just
// grabs relevant file paths.
bool BootCheck()
{
    int ret;
	char contentInfoPath[128];
	char usrdirPath[128];
    char patchPath[128];
    char modLoaderPath[128];

    // Since the game already did a boot check, calling the relevant functions of the
    // game causes a crash on real hardware, so as a temporary fix, just hardcode
    // to the gamedata folder, it's probably the only one you'd be using anyway.
    if (!Ib::IsEmulator())
    {
        cellGameGetParamString(CELL_GAME_PARAMID_TITLE, gTitleID, CELL_GAME_SYSP_TITLEID_SIZE);
        sprintf(usrdirPath, "%s/game/%s/USRDIR", "/dev_hdd0", gTitleID);

        gGameDataPath.Assign(usrdirPath);
        gBaseDir.Assign(usrdirPath);

        sprintf(modLoaderPath, "%s/cwml", gGameDataPath.c_str());
        gModLoaderPath.Assign(modLoaderPath);

        return true;
    }

    if ((ret = cellGameBootCheck(&gBootType, &gBootAttributes, &gGameDataSize, NULL)) < 0)
    {
        MMLogCh(DC_INIT, "cellGameBootCheck failed, ret = 0x%08X\n", ret);
        return false;
    }

    if (gBootAttributes & CELL_GAME_ATTRIBUTE_DEBUG)
        gBootAttributes |= CELL_GAME_ATTRIBUTE_APP_HOME;

    cellGameGetParamString(CELL_GAME_PARAMID_TITLE_ID, gTitleID, CELL_GAME_SYSP_TITLEID_SIZE);

    if ((ret = cellGameContentPermit(contentInfoPath, usrdirPath)) < 0)
    {
        MMLogCh(DC_INIT, "cellGameContentPermit failed, ret = 0x%08X\n", ret);
        return false;
    }

    if ((ret = cellGameDataCheck(gBootType, gTitleID, NULL) < 0))
    {
        MMLogCh(DC_INIT, "cellGameDataCheck failed, ret = 0x%08X\n", ret);
        return false;
    }

    if ((ret = cellGameContentPermit(contentInfoPath, usrdirPath)) < 0)
    {
        MMLogCh(DC_INIT, "cellGameContentPermit failed, ret = 0x%08X\n", ret);
        return false;
    }

    if (gBootAttributes & CELL_GAME_ATTRIBUTE_PATCH)
    {
        memset(&gGameDataSize, 0, sizeof(CellGameContentSize));
        if ((ret = cellGamePatchCheck(&gGameDataSize, NULL)) < 0)
        {
            MMLogCh(DC_INIT, "cellGamePatchCheck failed, ret = 0x%08X\n", ret);
            return false;
        }

        if ((ret = cellGameContentPermit(contentInfoPath, patchPath)) < 0)
        {
            MMLogCh(DC_INIT, "cellGameContentPermit failed, ret = 0x%08X\n", ret);
            return false;
        }

        if (gBootAttributes & CELL_GAME_ATTRIBUTE_DEBUG)
            sprintf(patchPath, "%s/game/%s/USRDIR", "/dev_hdd0", gTitleID);
        
        gGameDataPath.Assign(patchPath);
    }
    else if (gBootType == CELL_GAME_GAMETYPE_HDD)
    {
        if (gBootAttributes & CELL_GAME_ATTRIBUTE_DEBUG)
            sprintf(usrdirPath, "%s/game/%s/USRDIR", "/dev_hdd0", gTitleID);
        gGameDataPath.Assign(usrdirPath);
    }

    gBaseDir.Assign(usrdirPath);
    sprintf(modLoaderPath, "%s/cwml", gGameDataPath.c_str());
    gModLoaderPath.Assign(modLoaderPath);
    
    return true;
}