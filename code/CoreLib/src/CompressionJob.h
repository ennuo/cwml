#pragma once

#include <refcount.h>
#include <fifo.h>
#include <SerialiseEnums.h>
#include <compress.h>

enum ECompressionJobState {
    CJS_BEGIN,
    CJS_WORKING,
    CJS_DONE
};

class CompressionJob : public CBaseCounted {
public:
    CompressionJob(CMMSemaphore*, u32, u32);
    virtual ~CompressionJob();
    void EnqueueForCompress(bool, u32);
    bool IsFinished() const;
    ECompressionJobState GetState() const;
    u32 GetBytesRemaining() const;
    bool Empty() const;
    u16 GetInputBytes() const;
    u16 GetOutputBytes() const;
    void AddData(const void*, u32);
    void Finalise(ByteArray&);
public:
    static void CompressJob();
    CompressionJob(const CompressionJob& rhs);
    CompressionJob* operator=(const CompressionJob& rhs);
private:
    SCompressJobResult* CompressResult;
    u32 BufferSize;
    void* Buffer;
    void* BufferOut;
    u16 OriginalSize;
    u16 StoredSize;
    CMMSemaphore* Done;
    u32 JobID;
    u32 Level;
    ECompressionJobState State;
    CP<CompressionJob> Prev;
    CP<CompressionJob> NExt;
};

class DecompressionJob : public CBaseCounted {
public:
    DecompressionJob(CMMSemaphore* s, const CBaseVector<char>* v, u32 load_pos, u16 stored_size, u16 original_size);
    virtual ~DecompressionJob();
    void EnqueueForDecompress(bool spu_available, u32 tag);
    bool IsFinished() const;
    inline ECompressionJobState GetState() const { return State; }
    inline u32 GetBytesRemaining() const { return OriginalSize - Offset; }
    void GetData(void* data, u32 size);
    inline u16 GetBufferSize() const { return OriginalSize; }
    inline bool IsCompressed() const { return OriginalSize != StoredSize; }
    ReflectReturn Finalise();
public:
    static void DecompressJob(void*);
private:
    DecompressionJob(const DecompressionJob& rhs);
    DecompressionJob& operator=(const DecompressionJob& rhs);
private:
    SUncompressJobResult* UncompressResult;
    void* Buffer;
    u16 StoredSize;
    u16 OriginalSize;
    const CBaseVector<char>* Vector;
    CMMSemaphore* Done;
    u32 JobID;
    u32 LoadPos;
    u16 Offset;
    ECompressionJobState State;
public:
    CP<DecompressionJob> Next;
};
