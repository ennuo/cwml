#include <refcount.h>

CBaseCounted::~CBaseCounted()
{

}

#ifdef RESOURCE_SYSTEM_REIMPLEMENTATION
    StaticCPForm* gStaticCPHead;
#endif

#ifdef WIN32
    #include <windows.h>

    int CBaseCounted::AddRef()
    {
        return InterlockedIncrement((LONG*)&RefCount) - 1;
    }

    int CBaseCounted::Release()
    {
        return InterlockedDecrement((LONG*)&RefCount) + 1;
    }
#else
	#include <cell/atomic.h>

    int CBaseCounted::AddRef() 
    { 
        return cellAtomicIncr32((uint32_t*) &RefCount); 
    }

    int CBaseCounted::Release() 
    { 
        return cellAtomicDecr32((uint32_t*) &RefCount); 
    }

#endif


