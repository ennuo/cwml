#pragma once

#include <refcount.h>
#include <filepath.h>
#include <vector.h>
#include <ResourceDescriptor.h>
#include <SerialiseEnums.h>
#include <Serialise.h>

class CSerialisedResource : public CBaseCounted {
public:
    CSerialisedResource();
    CSerialisedResource(const CResourceDescriptorBase& desc);
    CSerialisedResource(const CSerialisedResource& rhs);
public:
    const CResourceDescriptorBase& GetDescriptor() const { return Descriptor; }
    ReflectReturn LoadCompressedBytes(CReflectionLoadVector& r);
private:
    CResourceDescriptorBase Descriptor;
public:
    CFilePath LoosePath;
    ByteArray Data;
};