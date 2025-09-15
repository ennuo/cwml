#ifdef VITA

#include <Ib/Hook.h>

tai_module_info_t gModuleInfo;

void Ib::Initialize()
{
    gModuleInfo.size = sizeof(tai_module_info_t);
    taiGetModuleInfo((const char*)TAI_MAIN_MODULE, &gModuleInfo);
}

void Ib::Close()
{

}





#endif // VITA