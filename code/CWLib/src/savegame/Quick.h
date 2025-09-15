#pragma once

#include <ResourceDescriptor.h>
#include <SerialiseEnums.h>
#include <UserUtils.h>
#include <filepath.h>

class SSaveKey {
public:
    inline SSaveKey() : Deprecated1(), Copied(false), RootType(RTYPE_INVALID), Deprecated2(), RootHash(), Deprecated3() {}
public:
    u32 Deprecated1[10];
    bool Copied;
    EResourceType RootType;
    u32 Deprecated2[3];
    CHash RootHash;
    u32 Deprecated3[10];
};

ReflectReturn CheckCache(const CFilePath& fart, SSaveKey& key, LocalUserID* user);
ReflectReturn CheckCache(FileHandle fd, SSaveKey& key, LocalUserID* user);