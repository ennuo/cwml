#include "ResourceDescriptor.h"
#include "GuidHashMap.h"
#include "SerialisedResource.h"

CSerialisedResource::CSerialisedResource(const CResourceDescriptorBase& desc) :
CBaseCounted(), Descriptor(desc), LoosePath(), Data()
{

}

CHash CResourceDescriptorBase::LatestHash() const
{
    if (!GUID) return Hash;
    const CFileDBRow* row = FileDB::FindByGUID(GUID);
    return row != NULL ? row->GetHash() : CHash::Zero;
}
