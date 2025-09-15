#pragma once

enum ECompressResult {
    CR_PENDING,
    CR_COMPLETED_FAILURE,
    CR_COMPLETED_SUCCESS
};

enum EUncompressResult {
    UR_PENDING,
    UR_COMPLETED_FAILURE,
    UR_COMPLETED_SUCCESS
};

struct SUncompressJobResult {
    EUncompressResult Result;
};

struct SCompressJobResult {
    ECompressResult Result;
    u32 StoredLen;
};
