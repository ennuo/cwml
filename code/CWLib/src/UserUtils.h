#pragma once

#ifdef WIN32
    typedef unsigned int CellSysutilUserId;
#else
    #include <sysutil/sysutil_common.h>
#endif

typedef CellSysutilUserId LocalUserID;