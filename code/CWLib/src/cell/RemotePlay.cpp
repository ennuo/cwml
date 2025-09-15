#include <sysutil/sysutil_subdisplay.h>
#include <sys/memory.h>


void CloseSubDisplay()
{

}

void SubDisplayHandler(int cbMsg, uint64_t cbParam, void *userdata )
{

}

bool InitSubDisplay()
{
    // supposed to check remote play in config, but womp
    CellSubDisplayParam param;
    param.version = CELL_SUBDISPLAY_VERSION_0001;
    param.nGroup = 1;
    param.nPeer = 1;

    param.videoParam.format = CELL_SUBDISPLAY_VIDEO_FORMAT_A8R8G8B8;
    param.videoParam.width = 480;
    param.videoParam.height = 272;
    param.videoParam.pitch = 512*4;
    param.videoParam.aspectRatio = CELL_SUBDISPLAY_VIDEO_ASPECT_RATIO_16_9;
    param.videoParam.videoMode = CELL_SUBDISPLAY_VIDEO_MODE_SETDATA;

    int mem_size = cellSubDisplayGetRequiredMemory(&param);
    if (mem_size < 0)
    {
        return false;
    }

    sys_memory_container_t container;
    if (sys_memory_container_create(&container, mem_size) < 0)
    {
        return false;
    }

    cellSubDisplayInit(&param, SubDisplayHandler, NULL, container);

    return true;





    return true;
}

void SubDisplayTransferImage(u32 x_res, u32 y_res, u32 offset)
{

}

bool SubDisplayWantTransferImage()
{
    return false;
}
