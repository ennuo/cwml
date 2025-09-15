#pragma once

#include <stdint.h>

namespace Ib
{
    enum ESHA1Result 
    {
        SHA1_SUCCESS,
        SHA1_NULL,
        SHA1_INPUT_TOO_LONG,
        SHA1_STATE_ERROR
    };

    struct SHA1_CONTEXT 
    {
        uint32_t  h0,h1,h2,h3,h4;
        uint32_t  nblocks;
        unsigned char buf[64];
        int  count;
    };

    class HashContext
    {
    public:
        inline HashContext()
        {
            Reset();
        }

    public:
        ESHA1Result Reset();
        ESHA1Result AddData(const uint8_t* message_array, unsigned int length);
        ESHA1Result Result(uint8_t* message_digest);
    private:
        int Computed;
        ESHA1Result Corrupted;
        SHA1_CONTEXT Context;
    };

};