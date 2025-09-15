#include <SerialiseCW.h>
#include <Resource.h>
#include <ResourceDescriptor.h>
#include <SharedSerialise.h>
#include <Variable.h>
#include <SerialiseVM.h>

template <typename R>
ReflectReturn ReflectResourceCP(R& r, CP<CResource>& d, EResourceType type)
{
    ReflectReturn rv;
    
    if (r.GetRevision() > 0x22e)
    {
        u32 flags = 0;
        if (d && r.GetSaving() && (d->GetGUID() || d->LoadedHash))
            flags = d->GetFlags() & (FLAGS_TEMPORARY | FLAGS_UNSHARED);
        
        if ((rv = Add(r, flags, "flags")) != REFLECT_OK)
            return rv;
    }
    
    CResourceDescriptorBase desc(type, 0);
    if (r.GetSaving() && d)
        d->GetLoadDescriptor(desc);

    if ((rv = ReflectDescriptor(r, desc, true, false)) != REFLECT_OK)
        return rv;
    
    if (r.GetLoading() && desc.IsValid())
    {
#ifndef LBP2 // TODO: FIX ME LATER!!
        CStreamPriority prio = r.GetLazyCPPriority();
        if (prio.FlagIsSet(STREAM_PRIORITY_DEFAULT) && GetStreamPriority(type) > STREAM_NO_STREAMING)
            prio.SetFlag(STREAM_PRIORITY_DONT_DESERIALISE, true);

        d = IsBrokenResource(desc) ? NULL : LoadResource(desc, prio, 0, false);
#endif
    }

#ifdef LBP1
    if (d) d->DependencyWalkStamp = -1;
#endif

    return rv;
}

// template ReflectReturn ReflectResourceCP<CReflectionLoadVector>(CReflectionLoadVector& r, CP<CResource>& d, EResourceType type);
// template ReflectReturn ReflectResourceCP<CGatherVariables>(CGatherVariables& r, CP<CResource>& d, EResourceType type);


template <typename R>
ReflectReturn ReflectResource(R& r, CResource& d)
{
    switch (d.GetType())
    {
#define RESOURCE_MACRO(type, class_name, headers) case type: return Reflect(r, (class_name&)d); break;
    #include <ResourceList.h>
#undef RESOURCE_MACRO
        default: return REFLECT_NOT_IMPLEMENTED;
    }
}

template <typename R, typename D>
ReflectReturn Reflect(R& r, CP<D>& d)
{
    return ReflectResourceCP(r, (CP<CResource>&)d, GetResourceType<D>());
}

#define RESOURCE_MACRO(type, class_name, headers) \
    template ReflectReturn Reflect<CReflectionLoadVector, class_name>(CReflectionLoadVector& r, CP<class_name>& d); \
    template ReflectReturn Reflect<CGatherVariables, class_name>(CGatherVariables& r, CP<class_name>& d);
    #include <ResourceList.h>
#undef RESOURCE_MACRO

template <typename R>
ReflectReturn SerialiseResourceSaveBinary(R& r, ByteArray& bytes, CResource& d, const CSerialiseControlParams& params, const char* hdr, const SResourceHeader& resource_header, CHash* ret_hash)
{
    return REFLECT_NOT_IMPLEMENTED;
}

ReflectReturn SerialiseResourceSaveText(CResource& d, ByteArray& bytes, CStreamPriority stream_priority_override, const char* hdr)
{
    CCSLock _the_lock(gResourceCS, __FILE__, __LINE__);
    CGatherVariables variables;

    switch (d.GetType())
    {
        #define RESOURCE_MACRO(type, class_name, headers) case type: Init(variables, (class_name*)&d); break;
            #include <ResourceList.h>
        #undef RESOURCE_MACRO
        default: return REFLECT_NOT_IMPLEMENTED;
    }

    variables.SetLazyCPPriority(stream_priority_override);
    return GatherVariablesSave(bytes, variables, true, hdr);
}

void ReadHeader(CResource& d, const ByteArray& bytes, SResourceHeader& resource_header) // 405
{
    if (bytes.size() < 4) return;
    if (!CheckTypeBy4ByteHeader(bytes.begin(), resource_header, d.GetType()) &&
        GetPreferredSerialisationType(d.GetType()) == PREFER_TEXT)
    {
        resource_header.IsText = true;
    }
}

ReflectReturn SerialiseResource(CResource& d, const CSerialiseControlParams& params, CHash* ret_hash) // 815
{
    if (!d.CSR || d.IsError() || d.IsLoading())
        return REFLECT_RESOURCE_IN_WRONG_STATE;

    CStreamPriority stream_priority_override; // 826
    ReflectReturn rv; // 836
    CP<CSerialisedResource> csr = d.CSR;
    ByteArray& bytes = csr->Data;
    
    // ESerialisationType st; // 880
    // SResourceHeader resource_header // 884
    // const char* hdr // 888
    // dummy_header

    if (params.Mode == MODE_LOAD)
    {
        if (d.IsLoaded())
            UnloadResource(&d);

        SResourceHeader header;
        ReadHeader(d, bytes, header);
        d.SetLoadState(LOAD_STATE_LOADING_DATA);

        SRevision revision(gFormatRevision, gFormatBranchDescription);
        if (!header.IsText)
        {
            CReflectionLoadVector r(&bytes);
            if (GetPreferredSerialisationType(d.GetType()) == PREFER_FILE)
                return REFLECT_NOT_IMPLEMENTED;

            if (header.Skip4)
            {
                u32 magic;
                Reflect(r, magic);
            }

            bool is_compressed = false;
            u8 compression_flags = 0;

            if (header.HasRevision)
            {
                if ((rv = Reflect(r, revision.Revision)) != REFLECT_OK) return rv;
                if ((rv = revision.CheckRevision()) != REFLECT_OK) return rv;

                u32 dependency_offset;
                if (revision.IsAfterRevision(gSelfDescribingDependencyRevision))
                {
                    if ((rv = Reflect(r, dependency_offset)) != REFLECT_OK) 
                        return rv;
                }

                if (revision.IsAfterRevision(gBranchDescriptionFormatRevision))
                {
                    if ((rv = Reflect(r, revision.BranchDescription)) != REFLECT_OK)
                        return rv;
                }

                if ((rv = revision.CheckBranchDescription()) != REFLECT_OK)
                    return rv;

                if (revision.IsAfterRevision(gCompressionFlagsRevision) && (rv = Reflect(r, compression_flags)) != REFLECT_OK)
                    return rv;

                if (revision.IsAfterRevision(gCompressedResourcesRevision) && (rv = Reflect(r, is_compressed)) != REFLECT_OK)
                    return rv;

                // if (header.IsEncrypted && (rv = r.Decrypt()) != REFLECT_OK)
                //     return rv;
            }
            else
            {
                is_compressed = true;
            }

            if (is_compressed)
                r.LoadCompressionData(NULL);

            r.SetRevision(revision);
            r.SetCompressionFlags(compression_flags);

            if ((rv = ReflectResource(r, d)) != REFLECT_OK)
            {
                r.CleanupDecompression();
                return rv;
            }

            if ((rv = r.CleanupDecompression()) != REFLECT_OK)
                return rv;
            
            if (header.HasRevision)
            {
                gMinRevision = MIN_MACRO(gMinRevision, revision.Revision);
                if (revision.IsAfterRevision(gSelfDescribingDependencyRevision))
                {
                    // if ((rv = ReadDepsData(d, r)) != REFLECT_OK)
                    //     return rv;
                }
            }
        }
        else
        {
            CCSLock lock(gResourceCS, __FILE__, __LINE__);
            CGatherVariables variables;

            switch (d.GetType())
            {
                #define RESOURCE_MACRO(type, class_name, headers) case type: Init(variables, (class_name*)&d); break;
                    #include <ResourceList.h>
                #undef RESOURCE_MACRO
                default: return REFLECT_NOT_IMPLEMENTED;
            }

            char magic[4] = {0};
            if ((rv = GatherVariablesLoad(bytes, variables, true, magic)) != REFLECT_OK)
                return rv;
        }

#ifndef LBP2 // todo: fix me later!!!!!!
        if ((rv = d.LoadFinished(revision)) != REFLECT_OK)
            return rv;
#endif

        d.SetLoadState(LOAD_STATE_LOADED);
        return REFLECT_OK;
    }

    if (!d.IsLoaded()) return REFLECT_RESOURCE_IN_WRONG_STATE;

    bytes.clear();
    ESerialisationType st = params.BinaryOrTextType == PREFER_DEFAULT ? 
        GetPreferredSerialisationType(d.GetType()) : params.BinaryOrTextType;

    SResourceHeader resource_header;
    resource_header.IsText = st == PREFER_TEXT;
    resource_header.IsEncrypted = st == PREFER_ENCRYPTED;
    const char* header = Get4ByteHeaderFromType(d.GetType(), resource_header);
    if (resource_header.IsText)
    {
        return SerialiseResourceSaveText(d, bytes, stream_priority_override, header);
    }
    else
    {
        // CReflectionSaveVector r(&bytes, params.CompressionLevel);
        // return SerialiseResourceSaveBinary(r, bytes, d, params, header, resource_header, ret_hash);
    }

    return REFLECT_NOT_IMPLEMENTED;
}
