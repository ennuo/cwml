#pragma once

#include <ReflectionVisitable.h>
#include <PartTypeEnum.h>
#include <PartList.h>

class CThing;

class CPart : public CReflectionVisitable {
public:
    CThing* GetThing() const { return Thing; }
private:
    CThing* Thing;
};
