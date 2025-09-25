#pragma once

#include <vector.h>

template <typename T, typename Allocator = CAllocatorMM>
class CParasiticVector : public CBaseVectorPolicy<T, Allocator> {
public:
    CParasiticVector() : CBaseVectorPolicy<T, Allocator>() {}
    CParasiticVector(T* data, u32 size, u32 max_size)
    {
        this->Data = data;
        this->Size = size;
        this->MaxSize = max_size;
    }

	CParasiticVector(CParasiticVector const& vec) : CBaseVectorPolicy<T, Allocator>()
	{
		*this = vec;
	}

	CParasiticVector& operator=(const CParasiticVector& vec)
	{
		if (&vec == this) return *this;

        this->Data = vec.Data;
        this->Size = vec.Size;
        this->MaxSize = vec.MaxSize;

		return *this;
	}
};