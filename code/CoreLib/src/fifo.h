#pragma once

#include <CritSec.h>
#include <vector.h>
#include <algorithm>

#ifdef WIN32
    #include <Windows.h>
#else
    #include <sys/synchronization.h>
#endif

class CMMSemaphore {
public:
    CMMSemaphore(int icount, int imaxcount);
    ~CMMSemaphore();
public:
    bool Increment();
    bool Increment(u32 value);
    bool WaitAndDecrement(int timeout);
    void DoAbort();
    bool Aborted() const;
private:
#ifdef WIN32
    HANDLE Sem;
#else
    sys_semaphore_t Sem;
#endif
    bool Abort;
};

template <typename T>
class CPriorityQueue {
struct SQueueEntry 
{
    inline SQueueEntry() : priority(), value() {}
    inline SQueueEntry(int prio, const T& v) : priority(prio), value(v) {}
    inline SQueueEntry(const SQueueEntry& rhs) : priority(), value() { *this = rhs; }

    inline SQueueEntry& operator=(const SQueueEntry& rhs)
    {
        priority = rhs.priority;
        value = rhs.value;
        return *this;
    }

    inline bool operator<(SQueueEntry& rhs) const { return priority < rhs.priority; }

    int priority;
    T value;
};

struct SCompareByPriority
{
    inline bool operator()(const SQueueEntry& lhs, const SQueueEntry& rhs) const
    {
        return lhs.priority < rhs.priority;
    }

    inline bool operator()(const SQueueEntry& lhs, int rhs) const
    {
        return lhs.priority < rhs;
    }

    inline bool operator()(int lhs, const SQueueEntry& rhs) const
    {
        return lhs < rhs.priority;
    }
};

struct Equals
{
    const T& Value;
    Equals(const T& value) : Value(value) {}
    inline bool operator()(const T& value) const
    {
        return Value == value;
    }
};

public:
    CPriorityQueue() : Q(), cs("PrioQ"), sem(0, 0) {}
public:
    inline void Abort() { sem.DoAbort(); }
    inline bool Aborted() const { return sem.Aborted(); }
    
    void Push(int priority, const T& value)
    {
        {
            CCSLock lock(&cs, __FILE__, __LINE__);
            SQueueEntry* index = std::lower_bound(Q.begin(), Q.end(), priority, SCompareByPriority());
            Q.insert(index, SQueueEntry(priority, value));
        }

        sem.Increment();
    }

    bool Peek(int& priority, T& value, bool pop)
    {
        CCSLock lock(&cs, __FILE__, __LINE__);
        if (Q.empty()) return false;

        const SQueueEntry& q = Q.front();
        priority = q.priority;
        value = q.value;

        if (pop)
            Q.erase(Q.begin());
        
        return true;
    }

    bool Pop(int& priority, T& value, int timeout)
    {
        if (!sem.WaitAndDecrement(timeout)) return false;
        return Peek(priority, value, true);
    }

    bool Empty()
    {
        return Q.empty();
    }

    template <typename Predicate>
    u32 Find(const Predicate& p, bool remove_if_found, bool set_new_prio, int new_prio)
    {
        CCSLock lock(&cs, __FILE__, __LINE__);

        SQueueEntry* it;
        for (it = Q.begin(); it != Q.end(); ++it)
        {
            if (p(it->value))
                break;
        }

        if (it == Q.end()) return -1;

        SQueueEntry q = *it;
        if (remove_if_found || set_new_prio)
            Q.erase(it);
        
        if (set_new_prio)
        {
            q.priority = new_prio;
            it = std::lower_bound(Q.begin(), Q.end(), new_prio, SCompareByPriority());
            Q.insert(it, q);
        }

        return it - Q.begin();
    }

    u32 Find(const T& find, bool remove_if_found, bool set_new_prio, int new_prio)
    {
        return Find(Equals(find), remove_if_found, set_new_prio, new_prio);
    }

    template <typename Predicate>
    u32 Erase(const Predicate& p)
    {
        return Find(p, true, false, 0);
    }

    u32 Erase(const T& value)
    {
        return Find(Equals(value), true, false, 0);
    }

    u32 ChangePriority(int new_prio, const T& find)
    {
        return Find(Equals(find), false, true, new_prio);
    }
    
    template <typename Predicate>
    u32 Count(const Predicate& p)
    {
        CCSLock lock(&cs, __FILE__, __LINE__);

        u32 count = 0;
        for (SQueueEntry* q = Q.begin(); q != Q.end(); ++q)
        {
            if (p(q->value))
                count++;
        }

        return count;
    }
private:
    CVector<SQueueEntry> Q;
    CCriticalSec cs;
    CMMSemaphore sem;
};
