#pragma once

#include <new>
#include "memory/Allocator.h"

template <typename T>
class CBaseVector {
public:
	typedef T* iterator;
public:
	inline CBaseVector() 
	{
		this->Data = NULL;
		this->Size = 0;
		this->MaxSize = 0;
	}

	inline bool empty() const { return this->Size == 0; }
	inline u32 size() const { return this->Size; }
	inline u32 max_size() const { return this->MaxSize; };
	inline u32 capacity() const { return this->MaxSize - this->Size; }

	inline T* begin() const { return Data; }
	inline T* end() const { return Data + Size; }
	inline T& operator[](int index) const { return this->Data[index]; }
	inline T& front() const { return this->Data[0]; }
	inline T& back() const { return this->Data[this->Size - 1]; }

	inline bool operator==(const CBaseVector<T>& rhs) const 
	{
		if (this == &rhs) return true;
		if (rhs.Size != Size) return false;
		return memcmp(Data, rhs.Data, Size) == 0;
	}

	inline bool operator!=(const CBaseVector<T>& rhs) const 
	{
		if (this == &rhs) return false;
		if (rhs.Size != Size) return true;
		return memcmp(Data, rhs.Data, Size) != 0;
	}

	inline u32& GetSizeForSerialisation() { return Size; }
protected:
	T* Data;
	u32 Size;
	u32 MaxSize;
};

template <typename T>
class CFixedVector : public CBaseVector<T> {
public:
	inline CFixedVector() : CBaseVector<T>() {}
	inline CFixedVector(T* data, u32 size) : CBaseVector<T>()
	{
		this->Data = data;
		this->Size = size;
		this->MaxSize = size;
	}

	inline CFixedVector(const CFixedVector<T>& rhs) : CBaseVector<T>()
	{
		*this = rhs;
	}

	inline CFixedVector<T>& operator=(CFixedVector<T> const& rhs)
	{
		this->Data = rhs.Data;
		this->Size = rhs.Size;
		this->MaxSize = rhs.MaxSize;

		return *this;
	}
};

template <typename T, typename Allocator = CAllocatorMM>
class CBaseVectorPolicy : public CBaseVector<T>  {
public:
	inline CBaseVectorPolicy() : CBaseVector<T>() {}
};

template <typename Allocator = CAllocatorMM>
struct AllocBehaviour {
public:
	static bool Reserve(void** data, u32& max_size, u32 new_max_size, u32 sizeof_t) 
	{
		if (max_size < new_max_size) 
		{
			u32 count = Allocator::ResizePolicy(max_size, new_max_size, sizeof_t);
			void* mem = Allocator::Realloc(*data, sizeof_t * count);
			if (mem != NULL) 
			{
				*data = mem;
				max_size = count;
				return true;
			}

			return false;
		}
		return true;
	}

	static bool Resize(void** data, u32& max_size, u32& size, u32 new_size, u32 sizeof_t) {
		bool reserved = AllocBehaviour<Allocator>::Reserve(data, max_size, new_size, sizeof_t);
		if (reserved) size = new_size;
		return reserved;
	}
};

#include <DebugLog.h>

template <typename T, typename Allocator = CAllocatorMM>
class CRawVector : public CBaseVectorPolicy<T, Allocator> {
public:
	inline CRawVector() : CBaseVectorPolicy<T, Allocator>() {}
	inline CRawVector(u32 capacity) : CBaseVectorPolicy<T, Allocator>()
	{
		try_reserve(capacity);
	}

	inline CRawVector(CRawVector<T, Allocator> const& vec) : CBaseVectorPolicy<T, Allocator>()
	{
		*this = vec;
	}

	inline ~CRawVector()
	{
		Allocator::Free(this->Data);
	}

	inline CRawVector<T, Allocator>& operator=(const CRawVector<T, Allocator>& vec)
	{
		if (&vec == this) return *this;
		
		resize(vec.Size);
		if (vec.Size != 0)
			memcpy(this->Data, vec.Data, vec.Size * sizeof(T));
		
		return *this;
	}

	inline void push_front(T const& element)
	{
		if (this->Size == this->MaxSize)
			this->try_reserve(this->Size + 1);
		memmove((void*)(this->Data + 1), (void*)this->Data, this->Size * sizeof(T));
		this->Data[0] = element;
		this->Size++;
	}

	inline void push_back(T const& element) 
	{
		if (this->Size == this->MaxSize)
			this->try_reserve(this->Size + 1);
		this->Data[this->Size++] = element;
	}

	inline T pop_back()
	{
		T& element = this->back();
		this->Size--;
		return element;
	}

	inline T* insert(T* it, T const& element)
	{
		int index = it - this->Data;

		if (this->Size == this->MaxSize)
		{
			u32 newsize = Allocator::ResizePolicy(this->Size, this->Size + 1, sizeof(T));
			T* data = (T*)Allocator::Malloc(newsize * sizeof(T));
			if (data == NULL) return NULL;

			memcpy(data, this->Data, index * sizeof(T));
			data[index] = element;
			memcpy(data + index + 1, it, (this->Size - index) * sizeof(T));

			Allocator::Free(this->Data);
			this->Data = data;
			this->Size++;

			return this->Data + index;
		}

		T t = element;
		memmove(it + 1, it, (size_t)((this->Data + this->Size) - it) * sizeof(T));
		*it = t;

		this->Size++;
		return it;
	}

	inline T* insert(T* it, const T* begin, const T* end)
	{
		const u32 count = end - begin;
		const u32 index = it - this->Data;

		this->resize(this->size() + count * sizeof(T));
		if (it != this->end())
		{
			memmove(
				this->Data + index + count,
				this->Data + index,
				(this->Size - index) * sizeof(T)
			);
		}
		
		memmove(this->Data + index, begin, count * sizeof(T));

		return this->Data + index;
	}

	inline T* erase(T* i) 
	{
		unsigned int return_index = i - this->Data;
		unsigned int copy_index = return_index + 1;
		
		if (copy_index < this->Size)
			memmove(this->Data + return_index, this->Data + copy_index, (this->Size - copy_index) * sizeof(T));
		
		this->Size--;
		return this->Data + return_index;
	}

	inline void clear()
	{	
		Allocator::Free(this->Data);
		this->Data = NULL;
		this->Size = 0;
		this->MaxSize = 0;
	}

	bool try_reserve(u32 new_max_size) 
	{
		return AllocBehaviour<Allocator>::Reserve((void**)&this->Data, this->MaxSize, new_max_size, sizeof(T));
	}

	bool try_resize(u32 new_size) 
	{
		return AllocBehaviour<Allocator>::Resize((void**)&this->Data, this->MaxSize, this->Size, new_size, sizeof(T));
	}

	void resize(u32 new_size) { try_resize(new_size); }
	void reserve(u32 new_max_size) { try_reserve(new_max_size); }

	void swap(CRawVector<T, Allocator>& rhs)
	{
		T* data = rhs.Data;
		u32 size = rhs.Size;
		u32 max_size = rhs.MaxSize;

		rhs.Data = this->Data;
		rhs.Size = this->Size;
		rhs.MaxSize = this->MaxSize;

		this->Data = data;
		this->Size = size;
		this->MaxSize = max_size;
	}
};

template <typename T, typename Allocator = CAllocatorMM>
class CVector : public CBaseVectorPolicy<T, Allocator> {
public:
	inline CVector() : CBaseVectorPolicy<T, Allocator>() {}
	inline CVector(u32 capacity) : CBaseVectorPolicy<T, Allocator>() {
		this->try_reserve(capacity);
	}

	inline CVector(CVector<T, Allocator> const& vec) : CBaseVectorPolicy<T, Allocator>()
	{
		*this = vec;
	}

	inline CVector<T, Allocator>& operator=(CVector<T, Allocator> const& vec)
	{
		if (this == &vec) return *this;

		for (int i = 0; i < this->Size; ++i)
			(this->Data + i)->~T();
		
		this->Size = 0;
		reserve(vec.Size);
		this->Size = vec.Size;

		for (int i = 0; i < this->Size; ++i)
			new (this->Data + i) T(vec[i]);
		
		return *this;
	}

	inline ~CVector() 
	{
		for (unsigned int i = 0; i < this->Size; ++i)
			(this->Data + i)->~T();
		Allocator::Free(this->Data);
	}

	inline T pop_back()
	{
		T element = this->back();
		(this->Data + this->Size - 1)->~T();
		this->Size--;
		return element;
	}

	inline void push_back(T const& element) 
	{
		if (this->Size == this->MaxSize)
			this->try_reserve(this->Size + 1);
		new (&this->Data[this->Size]) T();
		this->Data[this->Size++] = element;
	}

	inline T* insert(T* it, T const& element)
	{
		int index = it - this->Data;

		if (this->Size == this->MaxSize)
		{
			u32 newsize = Allocator::ResizePolicy(this->Size, this->Size + 1, sizeof(T));
			T* data = (T*)Allocator::Malloc(newsize * sizeof(T));
			if (data == NULL) return NULL;

			for (int i = 0; i < index; ++i)
			{
				new (data + i) T(this->Data[i]);
				(this->Data + i)->~T();
			}

			new (data + index) T(element);

			for (int i = index; i < this->Size; ++i)
			{
				new (data + i + 1) T(this->Data[i]);
				(this->Data + i)->~T();
			}

			Allocator::Free(this->Data);
			this->Data = data;
			this->MaxSize = newsize;
		}
		else
		{
			for (int i = index; i < this->Size; ++i)
			{
				new (this->Data + i + 1) T(this->Data[i]);
				(this->Data + i)->~T();
			}
			
			new (this->Data + index) T(element);
		}

		this->Size++;
		return this->Data + index;
	}

	inline T* erase(T* it) 
	{
		int index = it - this->Data;

		(this->Data + index)->~T();
		for (int i = index + 1; i < this->Size; ++i)
		{
			new (this->Data + i - 1) T(this->Data[i]);
			(this->Data + i)->~T();
		}
		
		this->Size--;
		return this->Data + index;
	}

	inline void clear()
	{
		for (unsigned int i = 0; i < this->Size; ++i)
			(this->Data + i)->~T();
		
		Allocator::Free(this->Data);
		this->Data = NULL;
		this->Size = 0;
		this->MaxSize = 0;
	}

	bool try_reserve(u32 new_max_size) 
	{
		if (this->MaxSize < new_max_size) 
		{
			u32 count = Allocator::ResizePolicy(this->MaxSize, new_max_size, sizeof(T));
			T* data = (T*) Allocator::Malloc(count * sizeof(T));
			if (data != NULL) 
			{
				for (u32 i = 0; i < this->Size; ++i)
				{
					new (data + i) T(this->Data[i]);
					(this->Data + i)->~T();
				}

				Allocator::Free(this->Data);

				this->MaxSize = new_max_size;
				this->Data = data;
				return true;
			}
			return false;
		}

		return true;
	}

	inline void resize(u32 new_size) { try_resize(new_size); }
	inline void reserve(u32 new_max_size) { try_reserve(new_max_size); }
	
	bool try_resize(u32 new_size) 
	{
		if (try_reserve(new_size)) 
		{
			for (int i = new_size; i < this->Size; ++i)
				(this->Data + i)->~T();
			for (int i = this->Size; i < new_size; ++i)
				new (this->Data + i) T();
			this->Size = new_size;
			return true;
		}

		return false;
	}
};

typedef CRawVector<char, CAllocatorMMAligned128> ByteArray;