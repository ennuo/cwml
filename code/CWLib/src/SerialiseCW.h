#pragma once

#include <Variable.h>
#include <Serialise.h>
#include <SerialiseResource.h>

class CLazyCPPriorityBlock {
public:
    inline CLazyCPPriorityBlock(CStreamPriority* pri, CStreamPriority _newpri) :
    r(pri), oldpri(*pri), newpri(_newpri)
    {
        *pri = newpri;
    }

    inline ~CLazyCPPriorityBlock()
    {
        *r = oldpri;
    }
private:
    CStreamPriority oldpri;
    CStreamPriority newpri;
    CStreamPriority* r;
};
