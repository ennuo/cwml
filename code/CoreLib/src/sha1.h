#pragma once

enum ESHA1Result {
    SHA1_SUCCESS,
    SHA1_NULL,
    SHA1_INPUT_TOO_LONG,
    SHA1_STATE_ERROR
};

struct SHA1_CONTEXT {
    u32  h0,h1,h2,h3,h4;
    u32  nblocks;
    unsigned char buf[64];
    int  count;
};

class CSHA1Context { // sha1.h: 55
public:
    inline CSHA1Context()
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

void SHA1(const uint8_t* buf, unsigned int len, uint8_t* digest);
